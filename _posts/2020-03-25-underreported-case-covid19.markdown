---
layout: post
title:  "How much is the COVID19 case underreported in Indonesia?"
comments: true
date:   2020-03-25 10:49:12 +0000
published: true
categories:
- Probability and statistics
tags:
- Bayesian inference
---

## Summary

Using the same model as the
[previous post](https://mfkasim91.github.io/2020/03/24/survival-rate-covid19-indonesia/),
(a model that takes into account delay in recovery and death), and assuming the
infection fatality rate (chance of fatality **if infected**) is
\\(0.66^{+0.13}\_{-0.13}\%\\) \[[1](https://www.medrxiv.org/content/10.1101/2020.03.09.20033357v1.full.pdf)\],
I found that the reported case is only \\(2.3^{+0.5}\_{-0.5}\%\\) of the total
infection.
With the total 790 reported case (as of 25 March 2020), there might be about
34,000 infections currently (which most of them are mild).

## Introduction

The analysis results from the previous post shows that the case survival rate
is about 71% or the case fatality rate (CFR) about 29%.
I need to reiterate that this number is defined as the chance of fatality
**if being confirmed positive** in Indonesia.
This is different from the infection fatality rate (IFR) which is the chance of
fatality **if being infected** in Indonesia.
The high calculated case fatality rate is most probably due to the reported cases
are heavily skewed towards severe cases and underreporting the mild case.

## Results

Calculating the infection fatality rate is much harder as it requires mass-testing
which is still a problem in Indonesia.
However, there has been an estimate of the infection fatality rate in China
is about \\(0.66^{+0.13}\_{-0.13}\%\\) (86% confidence interval).
By assuming the infection fatality rate is the same in Indonesia, we can estimate
how much the unreported infection in Indonesia.

With the infection fatality rate about 0.66%, most of the infected patients will
be recovered or even do not develop the symptoms.
So we can assume that the unreported infection in Indonesia is mostly dominated
by patients who will survive.
With this assumption, the reported fraction can be calculated by simply as

$$\begin{equation}
\mathrm{reported\_fraction} = \frac{\mathrm{estimated\_IFR}}{\mathrm{estimated\_CFR}}
\end{equation}$$

By putting \\(\mathrm{CFR} = 29^{+3}\_{-3}\%\\) and the assumed
\\(\mathrm{IFR} = 0.66^{+0.13}\_{-0.13}\\), we obtain the reported fraction is
only \\(2.3^{+0.5}\_{-0.5}\%\\) of the total infection.
This estimate is similar to the results of the study
[here](https://cmmid.github.io/topics/covid19/severity/global_cfr_estimates.html).
With the total 790 reported case (as of 25 March 2020), the number translates to
34,000 number of infections currently.
