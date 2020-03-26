---
layout: post
title:  "Which province should the COVID19 tests be focused?"
comments: true
date: 2020-03-26 11:56:33 +0000
published: true
categories:
- Probability and statistics
tags:
- Bayesian inference
- COVID19
---

## Summary

With the demographic-adjusted case fatality rate, I
predict the number of cases in every provinces in Indonesia, incorporating the
onset-to-death delay.
The highest density of predicted COVID19 cases (as of 26 March 2020) are located
in Jakarta (190 / 100,000 populations), DI Yogyakarta (23 / 100,000 populations),
Kepulauan Riau (21 / 100,000 populations), and Bali (20 / 100,000 populations).
The provinces that might have large fraction of unreported cases are
(outside Jawa) Bali (1.1% detected), Kepulauan Riau (1.1% detected),
(in Jawa) DI Yogyakarta (1.5% detected), Jawa Tengah (1.6% detected),
and Jawa Barat (1.7% detected).

## Data and assumptions

#### Data

* The mean incubation period is \\(5.6^{+0.6}\_{-0.6}\\) days \[[ref](https://doi.org/10.3390/jcm9020538)\]
* The mean onset-to-death period is \\(15^{+2.4}\_{-2.4}\\) days \[[ref](https://doi.org/10.3390/jcm9020538)\]
* The exponential factor is \\(0.164^{+0.008}\_{-0.008}\\) / days or double every 4.2 days (fitted data from 18-26 March 2020)
* Adjusted Case-Fatality-Ratio with Indonesian age demographic is about \\(0.82^{+0.63}\_{-0.63}\%\\)
(age-based CFR from \[[ref](https://doi.org/10.1101/2020.03.09.20033357 )\] and
demographic data from \[[ref](https://www.bps.go.id/publication/2016/11/30/63daa471092bb2cb7c1fada6/profil-penduduk-indonesia-hasil-supas-2015.html)\])

#### Assumptions

* The death number is measured **proportionally** in all provinces in Indonesia.
* The deceased patients are staying in the same province since the infection.
* The numbers above that are obtained from China's data is applicable for Indonesia.

## Introduction

The high case fatality rate (CFR) in my
[previous post](https://mfkasim91.github.io/2020/03/24/survival-rate-covid19-indonesia/)
is most likely because a large fraction of the reported cases are severe cases.
Milder cases, that are still infectious, might be heavily underreported.
By taking the CFR number from a study using China's data
\[[ref](https://doi.org/10.1101/2020.03.09.20033357 )\] and adjusting the number
with Indonesian's age demographic, we can predict how many infectious cases are
in Indonesia.

In this study, I use the number of deaths per province to predict the total
infectious cases in Indonesia.
As it is more likely for the severe cases to be reported than the milder
(and still infectious) cases,
the number of deaths should be closer to the actual number than the
confirmed case.

## Method

Adjusting the Indonesian demographic with the age-based Case Fatality Rate (CFR),
we can get the adjusted CFR for Indonesia, which is \\(0.82^{+0.63}\_{-0.63}\%\\).
This means for every 100,000 people that develop the onset of symptoms, there
might be 820 persons who might die because of COVID19 (or 2 millions over
267 millions Indonesian people).

Using the CFR number directly to predict the number of infectious case might
underestimate the number of cases, as there is a delay from a person developing the
symptoms to death.
Therefore, to get a more accurate number on the infectious case, we need to take
into account the onset-to-death delay.

From the reference \[[ref](https://doi.org/10.3390/jcm9020538)\], they found that
the onset-to-death period is about 15 days.
Which means if there is one death related to COVID19 in a province, then the person
should have been infected since 15 days before.
If there is one deceased person at one day and we assume the CFR in Indonesia is 0.82%,
then there might be 122 persons that have been infected, including the deceased one,
15 days before.

During those 15 days, those 122 persons are likely to infect the other without
being detected and allow the number of infected persons grow exponentially.
By fitting the data from [here](https://www.worldometers.info/coronavirus/country/indonesia/),
we get the number of cases doubles every 4.2 days.
So if 15 days before there are 122 persons already infected, it means there might
already be 1450 persons get infected in total on the day the first mortality
was recorded.

This is the upper estimate if we assume the onset-to-death period is always 15
days.
In fact, not every onset-to-death period is 15 days.
We can take an assumption that the onset-to-death is following the positive Laplace
distribution, as usually employed in SEIR model.
Using Laplace distribution with mean \\(\tau_d = 15\ \mathrm{days}\\), the estimated
number of cases per deaths in the same day can be calculated by

$$\begin{equation}
n_{case} = \frac{\kappa \tau_d + 1}{\eta_{CFR}} n_{deceased}
\end{equation}$$

where \\(\kappa\\) is the exponential factor, \\(\tau\_d\\) is the mean onset-to-death
period, and \\(\eta\_{CFR}\\) is the adjusted CFR for Indonesia.
Following this equation, it means there might already be **422 cases on the day
the first mortality was recorded**.
I use this number to estimate the number of infectious cases in every province in
Indonesia, multiplied by the mortality numbers.

## Results

#### Predicted case / populations

Using the mortality data from [BNPB](http://covid19.bnpb.go.id/), we can predict COVID19
cases per populations.
Here are the results of the predicted COVID19 cases per 100,000 populations
**only for provinces that has nonzero death cases**.

<img title="Predicted COVID19 case / 100,000 populations" src="{{ site.baseurl }}/assets/idcovid19/predicted-case-per-pop.png" width="100%"/>

We can see that **Jakarta** has much higher case density than other provinces.
The case density in Jakarta is predicted to be 190 cases / 100,000 populations
(or 19,000 if we assume there are 10 million people in Jakarta).
If we remove Jakarta from the graph, we can see the numbers for other provinces more
clearly.

<img title="Predicted COVID19 case / 100,000 populations without Jakarta" src="{{ site.baseurl }}/assets/idcovid19/predicted-case-per-pop-wo-jakarta.png" width="100%"/>

The top-3 provinces (outside Jakarta) that have the highest density of predicted cases
are **DI Yogyakarta, Bali, and Kepulauan Riau**.
Kepulauan Riau is the only province with less than 2 millions populations which has a recorded death.
That makes it the top-3 provinces (outside Jakarta) with the highest predicted case density.

#### Predicted case / confirmed case

We can also calculate the ratio between the predicted cases and the confirmed cases.
It can illustrate the severity of undertesting in the corresponding area.

<img title="Predicted COVID19 case / confirmed cases" src="{{ site.baseurl }}/assets/idcovid19/predicted-case-per-conf.png" width="100%"/>

From the plots above, we can see that the number of reported cases in **Bali and
Kepulauan Riau** might be significantly underreported (the actual number might be
more than 80 times larger than the reported number).
The predicted unreported fraction for provinces in Jawa is not as bad as those
two provinces, however, **Jawa Tengah, Jawa Barat, and DI Yogyakarta** is predicted
to have large portion of undetected cases (with the predicted number is about
40-50 times larger than the reported number).

(I have to say that Sulawesi Selatan, where I come from, is performing really well
in detecting the cases)
