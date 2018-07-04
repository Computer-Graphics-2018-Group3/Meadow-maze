实现了离屏 MSAA ，后期处理中应用了 Bilateral Filter 。
其中，在 screen_shader.fs 中可修改 Bilateral Filter 的 Kernel 值，
由于 Kernel 数过多会导致场景过于模糊，暂将 Kernel 数设为3。