
# Imzadi

`Imzadi` is a 3D game engine built on `DirectX11`, `XInput` and `XAudio2`.  Those familiar with `ST:TNG` will recognize the name.

Here's the latest screen-shot...

![snapshot](https://github.com/spencerparkin/Imzadi/blob/main/screen_shot.jpg?raw=true)

Clearly I'm not an artist.  Nevertheless, I did all the texture mapping, all the modeling, all the rigging, and all the animating.  But I also did all the C++ programming and math to bring that data into the engine, rendering and simulating it in real time.  Despite all that, though, no one in the gaming industry will hire me.

Note that a port to `DirectX12` is possible, but I've concluded that it would need to be done using an abstraction layer to take the place of my current use of `DirectX11`, and it would have a similar API to that of `DirectX11`.  Further, it's unlikely that I could do better than `DirectX11`, even if I used `DirectX12` under the hood.  In any case, it would be interesting to make an RHI that could switch between the two APIs, or even something like `Vulkan`.  That's all pie-in-the-ski, though, and I'm at a point where there are more pressing directions I want to take the project.