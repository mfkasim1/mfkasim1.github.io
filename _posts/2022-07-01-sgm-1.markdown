---
layout: post
title: "Score-based generative model tutorial 1st part: without SDE"
comments: true
date: 2022-07-01 17:04 +0100
published: true
categories:
- Deep learning
- Generative model
tags:
- PyTorch
- Implementation
- Neural network
---

#### Generative model

If you've had too much time on the internet lately, you probably have seen the text-to-image generators such as DALL-E 2,
Imagen, or CogView2.
One of the key building blocks of those systems are generative model.
Generative model is the part responsible to generate images that looks as realistic as possible.

How does a generative model works?
The generative model is trained with a set of real data (or image) as its training dataset.
Statistically speaking, the training dataset can be seen as data sampled from an unknown distribution.
During the training, this distribution is to be unveiled and if the training is successful, the generative model
can sample new data from this distribution.

The details on how a generative model unveil the distribution and sample data from this distribution are different
based on the type of the generative model.
One class that I would like to talk in this series of blog posts is the score-based generative model with 
stochastic differential equations, based on [this paper](https://arxiv.org/pdf/2011.13456.pdf).

#### Score-based generative model (without SDE)

Before continue reading this post, I highly recommend reading the great blog post about score-based generative model
by Yang Song [here](https://yang-song.github.io/blog/2021/score/).

Let's denote the real data (e.g. images, videos) as \\(\mathbf{x}\\) as a vector of \\(n\\) elements.
For example, if we are working with MNIST images i.e. grey-scale image (1 channel) with 28-by-28 pixels, \\(n=28\times 28=784\\).
What we want to learn in score-based generative model is the score function (i.e. gradients of log probability density function),
\\(\nabla_\mathbf{x} \mathrm{log}\ p(\mathbf{x})\\), given samples of the real data, \\(\mathbf{x}_{1..N}\\).
By learning the score function, we can sample from the distribution using sampling algorithms, such as Hamiltonian Monte Carlo.
If you're curious why learn the score function (i.e. the gradient) instead of the log probability density function directly,
you can find the answer in Yang Song's [blog post](https://yang-song.github.io/blog/2021/score/).

The learned score function \\(\mathbf{s}(\mathbf{x};\theta)\\) can be represented by a neural network that takes the data \\(\mathbf{x}\\) as the input
and produces an output with the same size as its input.
The neural network's parameters, \\(\theta\\), can be learned by minimizing the loss function,

$$\begin{equation}
\mathcal{L}(\theta) = \mathbb{E}_{\mathbf{x}\sim p(\mathbf{x})}\left[\left\lVert \mathbf{s}(\mathbf{x};\theta) - \nabla_\mathbf{x} \mathrm{log}\ p(\mathbf{x}) \right\rVert^2\right],
\end{equation}$$

where the expectation value \\( \mathbb{E}\_{\mathbf{x}\sim p(\mathbf{x})}\left[\cdot \right] \\) can be obtained by calculating the mean of 
the term inside the bracket using the training data.
However, in a usual setting, the true gradients of the log probability density functions \\(\nabla\_{\mathbf{x}} \mathrm{log}\ p(\mathbf{x})\\)
are not available in the training data.
[Hyvarinen (2005)](https://www.jmlr.org/papers/volume6/hyvarinen05a/hyvarinen05a.pdf) shows that the loss function above can be rewritten as

$$\begin{equation}
\mathcal{L}(\theta) = \mathbb{E}_{\mathbf{x}\sim p(\mathbf{x})}\left[\frac{1}{2} \left\lVert\mathbf{s}(\mathbf{x};\theta)\right\rVert^2 + \mathrm{tr}\left(\nabla_\mathbf{x} \mathbf{s}(\mathbf{x}; \theta)\right) + g(\mathbf{x})\right].
\end{equation}$$

In the equation above, only the first and second terms need to be computed during the training.
The term \\(g(\mathbf{x})\\) does not depend on the neural network parameters, \\(\theta\\), so it does not matter in the training.
The first term is simply half of the vector norm of the score function.
The second term, \\(\mathrm{tr}\left(\nabla\_\mathbf{x} \mathbf{s}(\mathbf{x}; \theta)\right)\\), is the trace (i.e. sum of the diagonal)
of the Jacobian matrix \\(\nabla\_\mathbf{x} \mathbf{s}(\mathbf{x}; \theta)\\).

#### Implementation

Let's take a simple dataset where the data \\(\mathbf{x}\\) is a vector of 2 elements generated from the swiss roll distribution.
```python
import torch
from sklearn.datasets import make_swiss_roll

# generate the swiss roll dataset
xnp, _ = make_swiss_roll(1000, noise=1.0)
xtns = torch.as_tensor(xnp[:, [0, 2]] / 10.0, dtype=torch.float32)
dset = torch.utils.data.TensorDataset(xtns)
```

<img title="Swiss-roll dataset" src="{{ site.baseurl }}/assets/220701-sgm/swiss-dataset.png" width="60%"/>

Now let's define the neural network that will learn the score function.
This is just a simple multi-layer perceptron with LogSigmoid activation function.
I used logsigmoid because of personal preference, you can also use ReLU.

```python
# score_network takes input of 2 dimension and returns the output of the same size
score_network = torch.nn.Sequential(
    torch.nn.Linear(2, 64),
    torch.nn.LogSigmoid(),
    torch.nn.Linear(64, 64),
    torch.nn.LogSigmoid(),
    torch.nn.Linear(64, 64),
    torch.nn.LogSigmoid(),
    torch.nn.Linear(64, 2),
)
```

After that, we can implement the first and second terms of the loss function below,

$$\begin{equation}
\mathcal{L}(\theta) = \mathbb{E}_{\mathbf{x}\sim p(\mathbf{x})}\left[\frac{1}{2} \left\lVert\mathbf{s}(\mathbf{x};\theta)\right\rVert^2 + \mathrm{tr}\left(\nabla_\mathbf{x} \mathbf{s}(\mathbf{x}; \theta)\right) + g(\mathbf{x})\right].
\end{equation}$$

Note that to implement the Jacobian, I am using the functions from ``functorch``.

```python
from functorch import jacrev, vmap

def calc_loss(score_network: torch.nn.Module, x: torch.Tensor) -> torch.Tensor:
    # x: (batch_size, 2) is the training data
    score = score_network(x)  # score: (batch_size, 2)
    
    # first term: half of the squared norm
    term1 = torch.linalg.norm(score, dim=-1) ** 2 * 0.5
    
    # second term: trace of the Jacobian
    jac = vmap(jacrev(score_network))(x)  # (batch_size, 2, 2)
    term2 = torch.einsum("bii->b", jac)
    return (term1 + term2).mean()
```

Everything is ready, now we can start the training. (In my computer, it will take about 10-15 minutes to finish.)

```python
# start the training loop
opt = torch.optim.Adam(score_network.parameters(), lr=3e-4)
dloader = torch.utils.data.DataLoader(dset, batch_size=32, shuffle=True)
for i_epoch in range(5000):
    for data, in dloader:
        # training step
        opt.zero_grad()
        loss = calc_loss(score_network, data)
        loss.backward()
        opt.step()
```

Once the neural network is trained, we can generate the samples using Langevin MCMC.

$$\begin{equation}
    \mathbf{x}_{i + 1} = \mathbf{x}_i + \varepsilon \mathbf{s}(\mathbf{x}; \theta) + \sqrt{2\varepsilon} \mathbf{z}_i
\end{equation}$$

where \\(\mathbf{z}\_i\sim\mathcal{N}(\mathbf{0}, \mathbf{I})\\) is a random number sampled from the normal distribution.

```python
def generate_samples(score_net: torch.nn.Module, nsamples: int, eps: float = 0.001, nsteps: int = 1000) -> torch.Tensor:
    # generate samples using Langevin MCMC
    # x0: (sample_size, nch)
    x0 = torch.rand((nsamples, 2)) * 2 - 1
    for i in range(nsteps):
        z = torch.randn_like(x0)
        x0 = x0 + eps * score_net(x0) + (2 * eps) ** 0.5 * z
    return x0

samples = generate_samples(score_network, 1000).detach()
```

And here are the samples:

<img title="Swiss-roll dataset (learned)" src="{{ site.baseurl }}/assets/220701-sgm/resampled-swiss-dataset.png" width="60%"/>

As we can see, the generative model can draw new samples from the swiss dataset without knowing the distribution explicitly.
In this part we don't use the SDE (Stochastic Differential Equations) yet because we're only working with small number of parameters (i.e. \\(n=2\\)).
However, for large number of parameters (e.g. images, waveforms), we need to use the SDE to produce good results.
In the next tutorial, we will go through the score-based generative model with SDE.

#### Summary

Summary for this part:

* Neural network: \\(n\\) input parameters, \\(\mathbf{x}\\), with \\(n\\) output parameters, \\(\mathbf{s}(\mathbf{x};\theta)\\).
* Training: minimize \\(\mathcal{L}(\theta) = \mathbb{E}\_{\mathbf{x}\sim p(\mathbf{x})}\left[\frac{1}{2} \left\lVert\mathbf{s}(\mathbf{x};\theta)\right\rVert^2 + \mathrm{tr}\left(\nabla\_\mathbf{x} \mathbf{s}(\mathbf{x}; \theta)\right)\right].\\)
* Samples generation: \\(\mathbf{x}_{i + 1} = \mathbf{x}\_i + \varepsilon \mathbf{s}(\mathbf{x}; \theta) + \sqrt{2\varepsilon} \mathbf{z}\_i\\) for \\(i=\\{0, ..., N\\}.\\) with \\(\mathbf{z}\_i\sim \mathcal{N}(\mathbf{0}, \mathbf{I})\\) and some small value of \\(\varepsilon\\) and reasonable value of \\(\mathbf{x}\_0\\).

The jupyter notebook for this tutorial can be found [here](https://github.com/mfkasim1/score-based-tutorial).
