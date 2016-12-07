---
layout: post
title:  "How many papers were submitted?"
comments: true
date:   2016-12-10 14:22:00 +0000
categories:
- Probability and statistics
tags:
- bayesian probability
- beta distribution
- beta function
- binomial distribution
- NIPS
---

As I mentioned in my [recent post](http://sp.mfkasim.com/2016/11/15/my-paper-at-nips-workshop-on-bayesian-optimization/),
my paper has been accepted for poster presentation at Bayesian Optimisation Workshop at NIPS 2016.
List of accepted paper has also appeared on the workshop's [website](https://bayesopt.github.io/accepted.html).
There are 26 papers accepted in total.
From the submission, my paper was given ID 12 and appears 9th on the list.

<a href="{{ site.baseurl }}/assets/accepted-papers.png"><img title="9 accepted papers" src="{{ site.baseurl }}/assets/accepted-papers.png"/></a>

Since the list of accepted papers is not ordered in alphabetical order (title nor author), I assume the list is ordered by
the submission ID.
I also assume that the submission ID is given by the order of submission.
The question is "*given the above information, estimate how many papers were submitted to BayesOpt 2016?*"

Let's denote \\(d=12\\) as the \\(d\\)-th paper submitted, \\(r=9\\) as the \\(r\\)-th paper accepted, \\(a=26\\) as the total accepted papers,
and \\(s\\) as the total papers submitted. It is clear enough to say that \\(s \geq a\\), as it is impossible to have more accepted papers than submitted papers.
Given those information, we want to calculate the probability of \\(s\\),

$$\begin{equation}
P(s|a,d,r) = \frac{P(a|d,r,s)P(s)}{\sum_{s_i=a}^{\infty} P(a|d,r,s_i)P(s_i)}.
\label{eq:bayes}
\end{equation}$$

In order to calculate \\(P(a|d,r,s)\\) from the equation above, we introduce a new variable, \\(\eta\\), the acceptance rate for large samples.
Given the acceptance rate, \\(\eta\\), and the total submissions, \\(s\\), we can calculate the probability of having number of accepted papers, \\(a\\), using binomial distribution,

$$\begin{equation}
\label{eq:a-s-eta}
P(a|s,\eta) = \left(\begin{array}{c} s \\ a \end{array}\right) \eta^a (1-\eta)^{s-a}.
\end{equation}$$

To get the probability distribution value of \\(\eta\\), we can use beta distribution with information that there are \\(r\\) papers accepted out of \\(d\\) submissions.
This is in similar form with the result on my [previous post](http://sp.mfkasim.com/2016/10/21/what-is-the-chance-ahok-wins-the-election-in-one-round/),

$$\begin{equation}
\label{eq:eta-d-r}
P(\eta | d,r)\ \mathrm{d}\eta = \frac{\eta^r (1-\eta)^{d-r}}{B(r+1,d-r+1)}\ \mathrm{d}\eta,
\end{equation}$$

where \\(B(\alpha, \beta)\\) is the [beta function](https://en.wikipedia.org/wiki/Beta_function).

Now we can use equation \eqref{eq:a-s-eta} and \eqref{eq:eta-d-r} to obtain

$$\begin{align}
\label{eq:a-d-r-s}
P(a|d,r,s) & = \int_0^1 P(a|s,\eta) P(\eta|d,r)\ \mathrm{d}\eta \nonumber \\
           & = \left(\begin{array}{c} s \\ a \end{array}\right) \frac{1}{B(r+1,d-r+1)} \int_0^1 \eta^{a+r} (1-\eta)^{s-a+d-r}\ \mathrm{d}\eta \nonumber \\
           & = \left(\begin{array}{c} s \\ a \end{array}\right) \frac{B(a+r+1, s-a+d-r+1)}{B(r+1, d-r+1)}.
\end{align}$$

Obtaining \\(P(a|d,r,s)\\), we can use the Bayes theorem in equation \eqref{eq:bayes} to estimate the number of submissions.
Assuming that the probability of having number of submissions, \\(s\\), is uniform from \\(a\\) to \\(\infty\\).
This is also the same prior assumption in [German tank problem](https://en.wikipedia.org/wiki/German_tank_problem).
As this is a very small number, we can denote it as \\(\Omega\\).
Thus,

$$\begin{align}
\label{eq:final-results}
P(s|a,d,r) & = \left(\begin{array}{c} s \\ a \end{array}\right) \frac{B(a+r+1, s-a+d-r+1)}{B(r+1, d-r+1)} \Omega
               \left[\sum_{s_i=a}^{\infty} \left(\begin{array}{c} s_i \\ a \end{array}\right) \frac{B(a+r+1, s_i-a+d-r+1)}{B(r+1, d-r+1)} \Omega \right]^{-1} \nonumber \\
           & = \left(\begin{array}{c} s \\ a \end{array}\right) B(a+r+1, s-a+d-r+1)
               \left[\sum_{s_i=a}^{\infty} \left(\begin{array}{c} s_i \\ a \end{array}\right) B(a+r+1, s_i-a+d-r+1) \right]^{-1}.
\end{align}$$

With the equation above, it is now possible to calculate and plot the probability distribution.
The probability distribution of number of submissions is shown below.

<a href="{{ site.baseurl }}/assets/submissions-probability.png"><img src="Probability distribution" src="{{ site.baseurl }}/assets/submissions-probability.png"/></a>

From the last equation, we can calculate the most probable number of submissions, expected number of submissions as well as its standard deviation.
The most probable number of submissions is \\(31\\), while the expected number of submissions is \\(36.5 \pm 9.4\\).
