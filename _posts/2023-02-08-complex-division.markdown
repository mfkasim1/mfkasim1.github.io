---
layout: post
title: "How complex can a division be?"
comments: true
date: 2023-02-07 11:19 +0000
published: true
categories:
- Numerical operations
tags:
- PyTorch
- Implementation
---

If you have ever doing programming in your life, performing a division can be as simple as `a / b`.
How difficult can it be? Turns out it can be quite difficult.

For many years doing computational research, I have been taking that simple operation for granted.
As most libraries, programming languages, and even modern processors have already implemented
the division operation, I (as a ML engineer/researcher) never thought that I have to implement the division
operation myself in a library as popular as pytorch.
However, the story changed when I was working with complex numbers in pytorch.

Let's say we have two complex numbers, \\(a=a_r + i a_i\\), and \\(b=b_r + i b_i\\).
To compute the complex division \\(a / b\\), we can revisit our high school maths:

$$\begin{equation}
\frac{a}{b} = \frac{a_r b_r + a_i b_i}{b_r^2 + b_i^2} + i \frac{a_i b_r - a_r b_i}{b_r^2 + b_i^2}.
\end{equation}$$

And the implementation of the division operation above in Python can be written as

```python
def complex_div(a, b):
    denom = b.real ** 2 + b.imag ** 2
    return (a.real * b.real + a.imag * b.imag) / denom + 1j * (a.imag * b.real - a.real * b.imag) / denom
```

Pretty straightforward, right? Now, with the implementation above, try put `a` and `b` to be `1e-170 + 1e-170j`,
you'll find python complaining about division by zero.
What happens?

The numbers `1e-170 + 1e-170j` can still be represented by 64-bits floating point numbers. The result
(which is supposed to be `1.0`) is also well within the range 64-bits float can represent.
However, in the intermediary of our calculation of `complex_div` above, we computed the square of `b.real` and
`b.imag`.
The problem is, the square of `1e-170` is already outside the range of 64-bits floating point, and the computer
just assumes it to be 0.
Therefore, the `denom` variable is stored as zero.
This is known as underflow.
The opposite is also possible with numbers `1e170 + 1e170j` where the square of that number is treated as infinite
by the computer. This is known as overflow.
In fact, the underflow/overflow in the intermediate calculations is one reason of the existence of some specific
functions such as `hypot`.

You might think that this problem is specific to my implementation above, but this is more common than you might think!
If you use any pytorch before version 2, you can find the problem with the complex division like above.
There are some similar problems that occurs as well, for example with `torch.abs` for complex numbers.

You might also think that it is very rare for common cases to work with numbers smaller than `1e-170`.
However, if you're working with 32-bits floating point (single precision), the limit is about `1e-23`.
Even worse, if you're working with half precision which is very common in machine learning, you can get the
underflow problem with `1e-4`!

**What is the solution?**

Having encountered this problem in pytorch v1.13, I raised this [issue](https://github.com/pytorch/pytorch/issues/92043)
and volunteered to push a fix.
What I thought is going to be a simple fix turns out to be lengthy discussions [here](https://github.com/pytorch/pytorch/pull/92539)
and [here](https://github.com/pytorch/pytorch/pull/93277) with some very supportive pytorch developers (such as @lezcano and @peterbell10).

There are several challenges in implementing the complex division operations to solve the underflow/overflow problem.
The first is, obviously, we have to find a mathematical expression that avoids the underflow/overflow in the intermediate calculations.
As pytorch is optimized for several devices, we also need to implement the solution in multiple ways. This is the second challenge.
Also, as complex division is relatively common operations, we cannot make the new implementation to be much slower than the naive
implementation.

The CPU and GPU implementation can follow numpy's implementation.
Basically, it rewrites the complex division to be
$$\begin{equation}
\frac{a}{b} = \begin{cases}
\frac{a_r + a_i (b_i / b_r)}{b_r + b_i (b_i / b_r)} + i \frac{a_i - a_r (b_i / b_r)}{b_r + b_i (b_i / b_r)}\ &\mathrm{if\ }|b_r| > |b_i| \\
\frac{a_r (b_r / b_i) + a_i}{b_r (b_r / b_i) + b_i} + i \frac{a_i (b_r / b_i) - a_r}{b_r (b_r / b_i) + b_i}\ &\mathrm{if\ }|b_r| \leq |b_i|.
\end{cases}
\end{equation}$$
By computing \\((b_r / b_i)\\) or \\((b_i / b_r)\\) beforehand, we can avoid the underflow or overflow problem in the denominator.

The solution above works with CPU or GPU kernel.
However, as modern CPUs typically offer SIMD (single instruction, multi data), we also need to implement the solution with SIMD instructions.
The problem is the solution above has 2 conditions and if we want to implement conditionals in SIMD, the computer has to compute both equations
first and then select which one to use.
This is going to give a performance penalty as it is going to waste 50% of its computational results.
My initial thought was to compute it like below,
$$\begin{equation}
\frac{a}{b} = \frac{a_r}{|b|} \frac{b_r}{|b|} + \frac{a_i}{|b|} \frac{b_i}{|b|} + i \left(\frac{a_i}{|b|} \frac{b_r}{|b|} - \frac{a_r}{|b|} \frac{b_i}{|b|}\right),
\end{equation}$$
where \\(|b| = \sqrt{b_r^2 + b_i^2}\\).
However, this solution requires a square-root computation which was deemed to be too expensive for common operators such as division.

An elegant solution was proposed by @peterbell10, which writes the equation to be
$$\begin{equation}
\frac{a}{b} = \frac{(a_r / c) (b_r / c) + (a_i / c) (b_i / c)}{(b_r / c)^2 + (b_i / c)^2} + i \frac{(a_i / c) (b_r / c) - (a_r / c) (b_i / c)}{(b_r / c)^2 + (b_i / c)^2},
\end{equation}$$
where \\(c=\mathrm{max}(|b_r|, |b_i|)\\).
By dividing all terms by \\(c\\), it drags away the denominator from the too-small or too-large regions into close to 1, avoiding the
underflow and overflow problems.
Moreover, this solution does not need conditional, so it can be executed efficiently with SIMD.

**Edge cases**

Writing the solutions above is just half of the work.
Another half of hard work is how to handle edge cases or cases that most people usually don't care about.
This is the irony.

What are the edge cases? These are, for example, division by 0, operations that involves infinite or nans, etc.
In machine learning, once your computation reaches infinite or nans, there is a high chance that further
computations will be useless.
Most of the interesting things do not happen in the edge cases.
However, this is what makes my hair falls out.

Handled improperly, edge cases can gives the wrong results.
For example, division by \\(b=\infty + i \infty\\) should produce 0 if the nominator is finite.
However, following the equation above, we can get `nan` as the results because of the \\((b_r / c)\\) calculation.

To handle the edge cases properly, we usually need to run various combinations of `inf`, `-inf`, `nan`, `0`, or even `-0`, and we need to
know what to expect as the results.
Most of the time, the expected results for these edge cases are quite obvious, such as any operations involving `nan` should produce
`nan`s.
Some cases however, the behaviour of the edge cases does not align with other behaviour. See [this](https://stackoverflow.com/questions/74798626/) for example.
In this division case, fortunately the edge cases can be handled by making a separate calculation when \\(c\\) is infinite which should produce zero.

In the end, implementing the numerically-stable division slows down the operation by 10% up to 160% compared to the naive (but unstable) implementation.

**What I learned**

So far I have been using these simple operations in libraries such as numpy and pytorch without really appreciating a lot of smart works done behind it.
A seemingly simple division operation can actually be so complex and challenging to fulfill various constraints.
