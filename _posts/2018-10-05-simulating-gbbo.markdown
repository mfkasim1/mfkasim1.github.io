---
layout: post
title:  "Simulating the Great British Bake Off"
comments: true
date:   2018-10-05 21:55:43 +0100
categories:
- Probability and statistics
tags:
- Simulation
- Monte Carlo
---

I have just been brought by my wife to watch a show called "The Great British
Bake Off". It is a show where at the beginning of each season, there are 12
bakers taking part. Each week, they show off their baking skills and their
cakes. The best baker in each week wins the star baker, while the unlucky one
in each week gets eliminated, no matter how good they were in the previous week.
By the end of the season, there are only 3 bakers left and the champion and the
runner-ups are decided simultaneously.
The question I am interested to see is, "would it be advantageous to have
mediocre but consistent performance or great skill but inconsistent
performance (in a good and a bad way)?"
Let's simulate it with Monte Carlo.

Suppose that the performance of a baker \\(i\\) is a random variable
\\(X_i \sim \mathcal{N}(\mu_i, \sigma_i)\\)
drawn from a Normal distribution with mean \\(\mu_i\\) and standard deviation
\\(\sigma_i\\). So if a person bakes many times, his/her average performance
would be close to \\(\mu_i\\). If he/she has low \\(\sigma_i\\), his/her
performance would be quite consistent. Otherwise, the performance would involve
a lot of surprise. Also assume that the average performance of all people are
distributed normally, \\(\mu_i \sim \mathcal{N}(0,1)\\).
So is the standard deviation, \\(\sigma_i \sim \mathcal{N}(0,1)\\).

To make the simulation easier, let's make a function to choose \\(n\\) best
bakers given the mean and standard deviation of some bakers.

{% highlight python %}
import numpy as np
def get_best_n_bakers(means, stds, n):
    size = means.shape[0]
    # get the performance of every person at one time
    performance = np.random.randn(size) * stds + means
    # only take the top n
    idx_topn = np.argsort(performance)[-n:]
    means_topn = means[idx_topn]
    stds_topn = stds[idx_topn]
    return means_topn, stds_topn
{% endhighlight %}

## One possible scenario

Now let's follow a possible scenario from the audition stage. The audition stage
consists of several processes, including baking. To simplify, just assume that
the top 12 bakers are chosen based on their performance on the audition.

Assume there are 100 participants in the
audition and the show only 12 best bakers during the audition to be taken to
the show. The average performance of a person is assumed to be distributed
normally. So is the standard deviation.

{% highlight python %}
means = np.random.randn(100)
stds = np.random.randn(100)
means_top12, stds_top12 = get_best_n_bakers(means, stds, 12)
{% endhighlight %}

Here's the performance characteristic of the chosen 12 bakers, in this case.

<div style="text-align:center"><img title="The performance of the 12 bakers"
src="{{ site.baseurl }}/assets/181005-performance-of-12.png"
width="500"/></div>

After each week, one unlucky baker will get eliminated until the final where
only 3 bakers left. Let's simulate that.

{% highlight python %}
means_topn, stds_topn = means_top12, stds_top12
# iterate per each week until only 3 bakers left
for n in range(11,2,-1):
    means_topn, stds_topn = get_best_n_bakers(means_topn, stds_topn, n)
{% endhighlight %}

The performance of the finalists look like below.

<div style="text-align:center"><img title="The performance of the 3 finalists"
src="{{ site.baseurl }}/assets/181005-performance-of-3.png"
width="500"/></div>

Looking at the picture above, the performance of the finalists might be very
different. One has a very large standard deviation (i.e. the green one) and
has a very small standard deviation, or a consistent performance (i.e. the
orange one). And the winner is ...

{% highlight python %}
# get the best baker
means_top, stds_top = get_best_n_bakers(means_topn, stds_topn, 1)
{% endhighlight %}

<div style="text-align:center"><img title="The performance of the winner"
src="{{ site.baseurl }}/assets/181005-performance-of-1.png"
width="500"/></div>

the one who has a consistent performance!

## Winners' characteristics

The previous section just presents one possible scenario. To get a good picture
on who has bigger chance in winning the show, let's run thousands of
simulations and visualize the performances from all scenarios.

{% highlight python %}
winner_means = []
winner_stds = []
for _ in range(10000):
    # let's assume there are about 100 participants in the audition
    means = np.random.randn(100)
    stds = np.random.randn(100)
    # audition
    means, stds = get_best_n_bakers(means, stds, 12)
    # elimination weeks
    for n in range(11, 2, -1):
        means, stds = get_best_n_bakers(means, stds, n)
    # final week
    means, stds = get_best_n_bakers(means, stds, 1)

    # save the winner performance
    winner_means.append(means[0])
    winner_stds.append(stds[0])
{% endhighlight %}

Here are the plots.

<div style="text-align:center"><img title="The performance of the winners"
src="{{ site.baseurl }}/assets/181005-performance-of-winners.png"
width="500"/></div>

The plot shows that almost all winners have average performance better than
people on average (of course) and most winners tend to have consistent
performance (i.e. small standard deviation).

The interesting thing is that if there are more participants in the audition,
the condition to win the show changes.

{% highlight python %}
winner_means = []
winner_stds = []
for _ in range(10000):
    # let's assume there are about 10000 participants in the audition
    means = np.random.randn(10000)
    stds = np.random.randn(10000)
    # audition
    means, stds = get_best_n_bakers(means, stds, 12)
    # elimination weeks
    for n in range(11, 2, -1):
        means, stds = get_best_n_bakers(means, stds, n)
    # final week
    means, stds = get_best_n_bakers(means, stds, 1)

    # save the winner performance
    winner_means.append(means[0])
    winner_stds.append(stds[0])
{% endhighlight %}

<div style="text-align:center"><img title="The performance of the winners (10000 participants in audience)"
src="{{ site.baseurl }}/assets/181005-performance-of-winners-2.png"
width="500"/></div>

In this case, it is clearly not an advantageous to have a very consistent
performance, unless you have very outstanding skill. In case where there are
about 10000 participants in the audition stage, you would need more than
average skills (not necessarily outstanding) and some bit of luck and surprise.

Any thoughts?
