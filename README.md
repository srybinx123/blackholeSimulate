# blackholeSimulate
这是一个模仿https://github.com/rossning92/Blackhole 项目的模拟器，但本项目是使用visual studio编写的，且使用vcpkg进行包管理。由于conan已经更新到2.x，而原项目还在使用1.x版本，我在根据原项目的README指导操作时，花了一番力气来处理conan版本造成的问题。

原项目在bilibili有[视频](https://www.bilibili.com/video/BV19a4y17721/)，我看到原作者一开始使用vcpkg进行的包管理，并且对于windows用户而言直接使用visual studio会省很多功夫，所以最终选择了vcpkg + visual studio的方案。比较可惜的是暂时没有进行更多的工作，仅仅是复现了原项目的功能，之后会尝试继续完善。

核心的shader代码也放到了[shadertoy](https://www.shadertoy.com/view/WflSRj)上，还是挺有意思的。

 - 2025.03.23   添加了多普勒效应的模拟
 - 2025.07.03   修复了多普勒红移和蓝移在camera roll不同取值时可能出现反转的问题
 - 2025.07.18   制作webGL版本，并发布创意工坊
   - [web demo](https://srybinx123.github.io/blackhole-WebGL/)
   - [steam 创意工坊](https://steamcommunity.com/sharedfiles/filedetails/?id=3519301874) 