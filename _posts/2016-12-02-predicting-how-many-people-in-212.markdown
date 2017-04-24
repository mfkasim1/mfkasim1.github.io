---
layout: post
title:  "Estimating how many people were in 212 prayer rally"
comments: true
date:   2016-12-02 22:14:12 +0000
categories:
- Simple math
tags:
- geometry
- simple math
---
Today (2nd December 2016) there were prayer rally to protest against Jakarta's chinese and christian governor, Ahok, that was accused of insulting Islam.
There has been many speculations about how many people were participating in the Friday prayer rally. Some says 2 million people, some even says 7 million.
I am intrigued to estimate how many people were participating the rally based on photos available on the internet and simple geometry.

First, let us see the satellite pictures of the venue where the rally took place. It was around National Monument (Monas) in Jakarta.
This is the map of Monas compiled from several images from Google Maps.

<img title="Monas map" src="{{ site.baseurl }}/assets/monas.png" width="250"/>

We are going to fill in some area on the map above that is filled with people, based on information from some pictures from the internet.
Here are the first images.

<img title="212 Rally - 01a" src="{{ site.baseurl }}/assets/212-pics-01a.png" width="250"/>
<img title="212 Rally - 01b" src="{{ site.baseurl }}/assets/212-pics-01b.png" width="250"/>
<img title="212 Rally - 01c" src="{{ site.baseurl }}/assets/212-pics-01c.png" width="250"/>

From these pictures, we can mark down some areas around the round fountain.

<img title="Monas markdown 01" src="{{ site.baseurl }}/assets/monas-markdown-01.png" width="250"/>

And now here are some pictures around Monas.

<img title="212 Rally - 02a" src="{{ site.baseurl }}/assets/212-pics-02a.png" width="250"/>
<img title="212 Rally - 02b" src="{{ site.baseurl }}/assets/212-pics-02b.png" width="250"/>

To make sense of the direction, it is reasonable to use [Istiqlal Mosque](https://en.wikipedia.org/wiki/Istiqlal_Mosque,_Jakarta) as reference.
On map, it is shown as big white square at the northeast of Monas. With that, we can mark some areas that were occupied by the people.
From these pictures, it seems that the southeast part of Monas was relatively less dense. So let's mark the area around Monas.

<img title="Monas markdown 02" src="{{ site.baseurl }}/assets/monas-markdown-02.png" width="250"/>

Now from the mask, we can estimate the area occupied by the people.
To estimate the area, we can calculate how many pixels are occupied by the mask below and later normalise it using the scale.
Click [here]({{ site.baseurl }}/assets/monas-mask.png) for the full size image.

<img title="Monas mask" src="{{ site.baseurl }}/assets/monas-mask.png" width="250"/>

The code to calculate the mask area in pixels is as below.

{% highlight matlab %}
img = imread('monas-mask.png'); % read the image
img = img(:,:,3); % read only the blue channel
sum(img(:) < max(img(:))); % calculate the mask pixels
{% endhighlight %}

The resulted area is 138495 pixels.
About the scale, Google Maps give scale of 100 m by 84 pixels, so 100 x 100 square meter is occupied by 7056 pixels.
I put the scale at bottom right of the map pictures.
With that scale, we can estimate the area to be \\( A=138495 \times \frac{(100)(100)}{7056} = 1.9628\times10^5\ \mathrm{m}^2 \\).

With the knowledge that the people were doing the prayer during the rally, a reasonable estimate for that is one person occupied about 0.5 square meter.
Thus, the estimate number of people during the rally was about
$$
N \approx \frac{A}{0.5\ \mathrm{m}^2} \approx 392559.
$$

Based on my estimation, there are about 400,000 people during the rally. It is really far from some claims that said it was 2 million or even 7 million.

The number is estimated from the pictures shown in this blog. If you have additional picture to refine my estimate, feel free to contact me.

**UPDATE**

There has been many inputs from other people that there are several regions are not covered on the map.
So I took the inputs and let's update the estimate.
One of my friend said that there were also people that were praying in Tugu Tani (green arrow below).
And another friend said that he was praying in a place I didn't mark and he witnessed there were still long line behind him (red arrow below).
Let's expand the mask.

<img title="Monas markdown 03" src="{{ site.baseurl }}/assets/monas-markdown-03.png" width="300"/>

<img title="Monas mask 02" src="{{ site.baseurl }}/assets/monas-mask-02.png" width="300"/>

Applying the same code as above (with appropriate file name), we obtain the pixels covered by the mask is 178244 pixels.
And for the space occupied by a person, there are some people say that it was really cramped during the prayer, and they estimated to be around 3 persons per square meter.
Let's take that number as the upper bound estimate.
Therefore, with the same scale as before, with 3 persons per square meter, it is estimated that there were about **757,840** people during the rally.
If we are using 2 persons per square meter as before, there were about **505,227** people.
