---
layout: post
title:  "Easy hypernetwork implementation with functorch"
comments: true
date: 2022-06-09 13:45 +0100
published: true
categories:
- Deep learning
tags:
- PyTorch
- Implementation
- Neural network
---

#### functorch.vmap

Recently PyTorch team just released [`functorch`](https://pytorch.org/functorch/stable/) where it contains operations for function transformations, just like 
[JAX](https://jax.readthedocs.io/en/latest/).
One function that I think under-appreciated is [`functorch.vmap`](https://pytorch.org/functorch/stable/generated/functorch.vmap.html#functorch.vmap)
which allows us to evaluate the input function along an additional dimension of the tensor inputs.
For example:
```python
import torch, functorch

a, b = torch.random((10, 5)), torch.random((10, 5))
dot2 = functorch.vmap(torch.dot, in_dims=0, out_dims=0)
print(dot2(a, b).shape)  # (10,)
```
In the code above, `dot2` operates `torch.dot` on each row (0-th dimension) of `a` and `b`, then stack the outputs on the 0-th dimension.
Simply speaking, the `vmap`-ed function is doing a loop of the operation along the 0-th dimension of the inputs.

Initially, I was also pessimistic about the vmap because most of the time I write my code by incorporating the batch dimensions,
so I had no need of using `functorch.vmap`.
However, I just recently found out that `functorch.vmap` can be very useful in implementing [HyperNetworks](https://arxiv.org/abs/1609.09106).

#### HyperNetwork

HyperNetwork is a type of deep network architectures that consists of 2 networks: hypernetwork and synthetic network.
The first network, hypernetwork, is a neural network that produces the weights of the synthetic network.
The weights produced by hypernetwork are then used by the synthetic network to make a prediction.
As the weights of the synthetic network are produced by the hypernetwork, they are not trainable.
Only the weights of the hypernetwork that are trainable.
The illustration of the hypernetwork can be seen in the image below.
A very nice talk about HyperNetwork and its applications can be watched [here](https://www.youtube.com/watch?v=KY9DoutzH6k).

<p align="center">
<img title="Hypernetwork illustration" src="{{ site.baseurl }}/assets/220609-hypernetworks/hypernetwork.png" width="300"/>
</p>

#### Implementating HyperNetwork in functorch

The difficulty in implementing hypernetwork arise when the input to the hypernetwork (i.e. \\(x\\) in the picture above)
is batched.
When \\(x\\) is batched, the parameters for the synthetic network are also batched.
With a naive implementation, we can evaluate the synthetic network with for-loop along the batch dimension.
However, this can be very slow because it involves Python loop.
A better way to implement this is using `functorch.vmap`.

The input to `functorch.vmap` is a function, while in PyTorch, we usually define the module as an object.
Fortunately, functorch also provides a function [`functorch.make_functional`](https://pytorch.org/functorch/stable/generated/functorch.make_functional.html)
that converts a PyTorch's module into a function.

In this implementation, we would like to define a `torch.nn.Module` that contains those two neural networks
(hypernetwork and synthetic network) and in its `forward` method, it receives `x` and `z` as its inputs and produces `h(x, z)`.
For simplicity, we assume all of the parameters of the synthetic network can be produced in one forward pass of the
hypernetwork.

```python
import torch
import functorch
import numpy as np


class HyperNetwork(torch.nn.Module):
    def __init__(self, hypnet: torch.nn.Module, synthnet: torch.nn.Module):
        # hypnet is the network that takes x and produces the parameters of synthnet
        # synthnet is the network that takes z and produces h
        super().__init__()
        s_func, s_params0 = functorch.make_functional(synthnet)

        # store the information about the parameters
        self._sp_shapes = [sp.shape for sp in s_params0]
        self._sp_offsets = [0] + np.cumsum([sp.numel() for sp in s_params0])

        # make the synthnet_func to accept batched parameters
        synthnet_func = functorch.vmap(synthnet_func)
        # a workaround of functorch's bug #793
        self._synthnet_batched_func = [synthnet_func]
        self._hypnet = hypnet

    def forward(self, x: torch.Tensor, z: torch.Tensor) -> torch.Tensor:
        # x: (batch_size, nx), z: (batch_size, nz)
        params = self._hypnet(x)  # params: (batch_size, nparams_tot)

        # rearrange params to have the same shape as the synthnet params, except on the batch dimension
        params_lst = []
        for i, shape in enumerate(self._sp_shapes):
            j0, j1 = self._sp_offset[i], self._sp_offset[i + 1]
            params_lst.append(params[..., j0:j1].reshape(-1, *shape))

        # apply the function to the batched parameters and z
        h = self._synthnet_batched_func(params_lst, z)
        return h
```

Let's investigate it line-by-line in the initialization of the module.
First, the line
```python
s_func, s_params0 = functorch.make_functional(synthnet)
```
decomposes the synthetic network module into its function form, `s_func`, and its
parameters, `s_params0`, as a list of tensors.
Next, the lines
```python
# store the information about the parameters
self._sp_shapes = [sp.shape for sp in s_params0]
self._sp_offsets = [0] + np.cumsum([sp.numel() for sp in s_params0])
```
store information about the synthetic network parameters (`s_params0`). Specifically, we save the shape of each parameter
and the cumulative sums of the number of elements of the parameters in `self._sp_offset`.
If all of the parameters are lined up, the `i`-th parameter can be accessed in the index 
`[self._sp_offset[i]:self._sp_offset[i + 1]]`.

After we're done with the synthetic network parameters, we do the processing of the function form next.
```python
# make the synthnet_func to accept batched parameters
synthnet_func = functorch.vmap(synthnet_func)
```
The line above makes the `synthnet_func` able to receive batched parameters by applying `vmap` to the
function form of the synthetic network.
This is the most important step as we enable the synthetic network to process the parameters and the input to 
the synthetic network in a batched manner.
Then, the next lines
```python
# a workaround of functorch's bug #793
self._synthnet_batched_func = [synthnet_func]
self._hypnet = hypnet
```
save the function form of synthetic network and the hypernetwork module.
An interesting point to make is, by the time of writing (09 June 2022), we need to encapsulate
the functional form of synthetic network to prevent it being registered as a submodule in
`torch.nn.Module`.
Otherwise, it could raise an error if we're trying to move the parameters of the module to another device.
The details about this issue can be read [here](https://github.com/pytorch/functorch/issues/793).

In the forward method, the hypernetwork generates the parameters of synthetic networks in the line
```python
params = self._hypnet(x)  # params: (batch_size, nparams_tot)
```
The `params` variable is a tensor that lines up all synthetic network parameters in its last axis.
As all of the parameters of the synthetic network is collected in one tensor, we need to split it up.
The lines
```python
# rearrange params to have the same shape as the synthnet params, except on the batch dimension
params_lst = []
for i, shape in enumerate(self._sp_shapes):
    j0, j1 = self._sp_offset[i], self._sp_offset[i + 1]
    params_lst.append(params[..., j0:j1].reshape(-1, *shape))
```
re-arrange the `params` into a list of parameters like in `self._sp_shapes`, but
with additional batch dimension in the 0-th dimension.
After it's being re-arranged, we can just put the list of batched parameters
into the synthetic network function form in the line below as well as the input to the
synthetic network, `z`.
```python
h = self._synthnet_batched_func(params_lst, z)
return h
```

After defining the class above, there is just one problem left: defining the hypernetwork and synthetic
network.
The architecture of those networks depends on the problem you are trying to solve.
For simplicity, we can just define those networks as a simple 2-layer perceptrons as shown below.
Let's assume `x` is a 3-elements vector, `z` is a 2-elements vector, and the output `h` is a 5 elements vector.

```python
synthnet = torch.nn.Sequential(
    torch.nn.Linear(2, 100),  # nz = 2
    torch.nn.ReLU(),
    torch.nn.Linear(100, 5),  # nh = 5
)

# compute the total number of parameters in the synthetic network
_, synthnet_params = functorch.make_functional(synthnet)
n_synthnet_params = sum([p.numel() for p in synthnet_params])

hypnet = torch.nn.Sequential(
    torch.nn.Linear(3, 100),  # nx = 3
    torch.nn.ReLU(),
    torch.nn.Linear(100, n_synthnet_params),
)
module = HyperNetwork(hypnet, synthnet)
```

One thing to note is the lines below.
```python
# compute the total number of parameters in the synthetic network
_, synthnet_params = functorch.make_functional(synthnet)
n_synthnet_params = sum([p.numel() for p in synthnet_params])
```
As the output of hypernetwork is the parameteres of the synthetic network, we need to know
how many parameters in total there are in the synthetic network.
And the line above is to calculate the total number of parameters in the synthetic network.

Putting them together, we have the code below, in only about 50 lines of code!

```python
import torch
import functorch
import numpy as np


class HyperNetwork(torch.nn.Module):
    def __init__(self, hypnet: torch.nn.Module, synthnet: torch.nn.Module):
        # hypnet is the network that takes x and produces the parameters of synthnet
        # synthnet is the network that takes z and produces h
        super().__init__()
        s_func, s_params0 = functorch.make_functional(synthnet)

        # store the information about the parameters
        self._sp_shapes = [sp.shape for sp in s_params0]
        self._sp_offsets = [0] + np.cumsum([sp.numel() for sp in s_params0])

        # make the synthnet_func to accept batched parameters
        synthnet_func = functorch.vmap(synthnet_func)
        # a workaround of functorch's bug #793
        self._synthnet_batched_func = [synthnet_func]
        self._hypnet = hypnet

    def forward(self, x: torch.Tensor, z: torch.Tensor) -> torch.Tensor:
        # x: (batch_size, nx), z: (batch_size, nz)
        params = self._hypnet(x)  # params: (batch_size, nparams_tot)

        # rearrange params to have the same shape as the synthnet params, except on the batch dimension
        params_lst = []
        for i, shape in enumerate(self._sp_shapes):
            j0, j1 = self._sp_offset[i], self._sp_offset[i + 1]
            params_lst.append(params[..., j0:j1].reshape(-1, *shape))

        # apply the function to the batched parameters and z
        h = self._synthnet_batched_func(params_lst, z)
        return h

synthnet = torch.nn.Sequential(
    torch.nn.Linear(2, 100),
    torch.nn.ReLU(),
    torch.nn.Linear(100, 5),
)

# compute the total number of parameters in the synthetic network
_, synthnet_params = functorch.make_functional(synthnet)
n_synthnet_params = sum([p.numel() for p in synthnet_params])

hypnet = torch.nn.Sequential(
    torch.nn.Linear(3, 100),
    torch.nn.ReLU(),
    torch.nn.Linear(100, n_synthnet_params),
)
module = HyperNetwork(hypnet, synthnet)
```

#### Closing remark

The implementation above is a simplistic implementation of hypernetwork.
You can make it more complicated, for example, having more complicated architectures, or generating the parameters in multiple forward
passes as done in [this paper](https://arxiv.org/abs/1609.09106).
This post is just to show the capability of `functorch` and how `vmap` makes it easier to implement hypernetwork.
