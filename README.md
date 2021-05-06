# TTN_separator
Juce-based Tonal, Transient and Noise separation audio plugin

Algorithm based on
> D.Fitzgerald, "Harmonic/Percussive Separation using Median Filtering", in Proc. 13td Int. Conf. Digital Audio Effects (DAFx-10)

Separation methodology based on
> E. Moliner, J. Rämö, and V. Välimäki,, "Virtual Bass System with Fuzzy Separation of Tones and Transients", in Proc. 23rd Int. Conf. Digital Audio Effects (DAFx-20), Vienna, Austria, Sept. 8–12

With the noise factor (NF) defined as

![equation](https://latex.codecogs.com/svg.latex?R_\text{n}(m,k)=1-\left|R_\text{s}(m-k)-R_\text{t}(m,k)\right|^{\frac{1}{NF}}) 

Testing key pair
