---
layout: post
title:  "Estimating the number of COVID-19 cases in Indonesia"
comments: true
date: 2020-05-01 16:32:19 +0100
published: true
categories:
- Probability and statistics
tags:
- Bayesian inference
---

## Background

COVID-19 is well-known to have substantial portion of asymptotic infected person,
which makes the detection hard.
However, knowing the actual number of infected persons could inform how close we
are to the herd immunity.
One way to estimate the number of infection is by using the
infection-fatality-ratio (IFR) and the death number.

## Method



#### Inferring the gradient of the curve

Denote the new daily infection cases at a given day \\(t\\) as \\(y(t)\\).
The new daily infection cases in the logscale is assumed to follow the normal
distribution with standard deviation \\(\sigma\\),

$$\begin{equation}
\log\left[y(t)\right] \sim \mathcal{N}\left({a + \int b(t)\ \mathrm{d}t}; \sigma\right)
\end{equation}$$

where \\(a\\) is a constant indicating the initial condition and \\(b(t)\\) is
the gradient of the curve in the logscale.
The value of \\(b(t) > 0\\) indicates growing infection while \\(b(t) < 0\\)
shows that the infection has slowed down.
The success of lockdown or any suppression strategy is based on the value of
this gradient, \\(b(t)\\), if it has value less than 0.

The prior of parameters \\(a\\) and \\(\sigma\\) are given below,

$$\begin{align}
\log(\sigma) &\sim \mathrm{Uniform}(-8, 1) \\
a &\sim \mathrm{Uniform}(-2, 3).
\end{align}$$

The prior of the gradient is assumed to be Gaussian Process (GP) with squared
exponential kernel with mean 0,

$$\begin{align}
b(t) &\sim \mathcal{GP}\left[0; s^2 k(t,t')\right] \\
k(t,t') &= \exp\left[-(t-t')^2 / (2 l^2)\right].
\end{align}$$

The prior of the parameters in the GP are given below,

$$\begin{align}
s &\sim \mathrm{Uniform}(0,1) \\
\log(l) &\sim \mathrm{Uniform}(0, 3)
\end{align}$$

Having specified the model and the priors, we can infer the parameters above
using Markov Chain Monte Carlo (MCMC).
This is easily done using the differentiable probabilistic programming, such as
[Pyro](http://docs.pyro.ai/en/stable/).

#### Getting the number of tests

If the number of tests in a place is given, then this subsection is not needed.
However, for Indonesia, only the total number of tests per day is given, without
the details on the number of tests done for each province.
Fortunately, the number of positive cases for each province is published every
day, so we can use the information to infer the number of tests performed for
each province.

Denote the total number of tests (nationally) as \\(n_{tt}\\), the total
confirmed cases in a day as \\(n_{pt}\\), and the number of positive cases in a
province as \\(n_p\\).
From those information, we need to infer the number of tests done for the
province, \\(n_t\\).
Using the Bayes' theorem, the probability mass function (pmf) of the number of
tests for the province can be written as

$$\begin{equation}
p(n_t | n_p, n_{pt}, n_{tt}) = \frac{p(n_p | n_t, n_{pt}, n_{tt}) p(n_t | n_{pt}, n_{tt})}{\sum_{n_{t'}}p(n_p | n_{t'}, n_{pt}, n_{tt}) p(n_{t'} | n_{pt}, n_{tt})}.
\end{equation}$$

The likelihood \\(p(n_p | n_t, n_{pt}, n_{tt})\\) is assumed to take the
[hypergeometric distribution](https://en.wikipedia.org/wiki/Hypergeometric_distribution)
while the prior \\(p(n_t | n_{pt}, n_{tt})\\) is assumed to be uniform for
\\(0 \leq n_t \leq n_{tt}\\).
Using the likelihood and the prior forms, we can calculate the posterior
distribution of the number of tests done for a province.

#### Putting them altogether

To draw a meaningful conclusion if the spread has been really suppressed,
the inferred gradient \\(b(t)\\) must fulfil the following conditions:
(1) it must be less than 0, and (2) if it is less than 0 and the number of
tests is decreasing, then the gradient \\(b(t)\\) must be less than the
decreasing factor of the number of tests.

## Results

The results can be seen on [this page]({{ site.baseurl }}/idcovid19/)
