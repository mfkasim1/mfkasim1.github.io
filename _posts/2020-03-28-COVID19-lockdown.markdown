---
layout: post
title:  "Indonesian lockdown for COVID19"
comments: true
date: 2020-03-28 23:11:48 +0000
published: true
categories:
- Simulation
tags:
- COVID19
---

## Introduction

There are two questions I would like to answer in this post:

1. What is the impact of delaying lockdown due to COVID19 in Indonesia?
2. How big is the role of the people in reducing the severity of COVID19 in Indonesia?

## Method

I performed simulation based on the website [here](http://gabgoh.github.io/COVID/index.html?CFR=0.0082&D_hospital_lag=5&D_incbation=5.2&D_infectious=7.44&D_recovery_mild=11.1&D_recovery_severe=28.6&I0=1&InterventionAmt=0.3&InterventionTime=101&P_SEVERE=0.2&R0=3.84&Time_to_death=8.11&logN=19.39) with fitted parameters
to match Indonesia's data.
It is assumed that the basic reproduction number (\\(R\_0\\)) is about 3.86,
which means 1 person can infect 3.86 other persons on average.
This is the number calculated from [here](https://www.medrxiv.org/content/10.1101/2020.03.03.20030593v1)
for China's data, because there has been no a proper estimate for Indonesia's
\\(R\_0\\).
The simulation allows us to input the basic reproduction number before and after
an intervention is applied to see the effect of the intervention.

After the lockdown in Wuhan with other strategy implemented by the Chinese government,
the basic reproduction number can be reduced to 0.32 \[[ref](https://www.medrxiv.org/content/10.1101/2020.03.03.20030593v1)\].
For some reasons however, the lockdown in Wuhan is hardly be replicated by
other countries, as we have seen in Italy.
If we assume that Indonesia can achieve 90% effectiveness of the lockdown in Wuhan,
then the \\(R\_0\\) can be estimated to \\(R\_0 = (0.9)(0.32) + (1-0.9)(3.86) = 0.65\\).

The above value of \\(R\_0\\) assumes that 100% people comply with the lockdown rules.
However, in a non-ideal case where the only \\(f\%\\) of the people complies with
the lockdown rules (e.g. not doing gathering, etc), the basic reproduction number
after the lockdown can be approximated as \\(0.65 * f + 3.86 * (1-f)\\).

## Results

#### Impact of delaying the lockdown

With the number of confirmed COVID19 case exceed 1 thousand, it is almost
impossible to contain the viruses without having the lockdown.
In the worst case where there is no lockdown implemented, it is estimated that
more than **2 millions people** in Indonesia would die because of COVID19.
Implementing lockdown earlier would **significantly** reduces the number of deaths
in Indonesia.
Here is the simulation result of the number of deaths by the end of epidemic
as a function of time when lockdown is implemented.
This assumes that Indonesia can get the same effectiveness as the lockdown
strategy in Wuhan and all people comply with the rules all the time.

<img title="Number of deaths vs time when lockdown is implemented" src="{{ site.baseurl }}/assets/idcovid19/deaths-vs-time-lockdown.png" width="80%"/>

We can see that the total number of deaths grows exponentially as the lockdown is delayed.
Let's take an example.
If the lockdown is implemented on 30 March, the total number of deaths would be around 1600 people.
If it is implemented a week later (6 April), the total deaths would be about 3100 people.
Delaying it until for 2 weeks until 13 April would raise the number to 9900 people.
And if it is delayed until a month later, it would result in about 128000 deaths.
The longer it is delayed, the more people are going to die.
Remember, delaying the lockdown by 4 days would **double the number of deaths**.

I need to reiterate that this assumes that **all people comply** with the rules all the time
and Indonesia can reach about 90% effectiveness as Wuhan's lockdown strategy.

#### The role of people in having a successful lockdown

With 90% effectiveness of lockdown strategy, we will see how important the role
of people in making the lockdown successful is.
Let's say the lockdown is implemented on 30 March.
Here is the graph of the total number of deaths vs the fraction of population
that obeys the lockdown.

<img title="Number of deaths vs fraction of population obeying the rules" src="{{ site.baseurl }}/assets/idcovid19/deaths-vs-obey.png" width="80%"/>

We can see if the number of people disobey the rule increases, the number of deaths
**increases very significantly**.
The effect is much more significant than delaying the lockdown.
If all people obeys the lockdown's rules, the number of deaths would be 3700 by the end of epidemic.
If 5% of the population does not obey the rules (1 in 20 of your friends/colleagues), the number increases to 6000 deaths.
With 10% population does not follow the rules (1 in 10), the number of deaths increases significantly to 37000.
And if 15% of the population does not obey the lockdown's rules, the number sharply increases to 500 thousands deaths.

So, remember, if the government applies lockdown, please obey the rules.
**If not, hundreds of thousand people could die because of your action.**
