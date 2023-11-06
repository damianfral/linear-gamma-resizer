# linear-gamma-resizer

An quick and dirty C application that uses the VIPS library to resize given
images using linear gamma into three different sizes (3840x2160, 1920x1080 and
480x270) and save them in the AVIF format.

```shell
nix run github:damianfral/linear-gamma-resizer -- img/*.jpg
```
