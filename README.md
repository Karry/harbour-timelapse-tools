# TimeLapse tools for Sailfish OS

Sailfish OS application for capturing and assembling time-lapse videos directly on the phone.
Images may be captured by one of phone cameras, or external camera connected via USB OTG
dongle. Video may be assembled later from picture series. Video assembly is time and CPU
consuming. It is recommended to run it with connected charger. For more advanced 
image preprocessing (align horizon, crop, change contrast...), you may copy series 
to computer, use some image processing tool (for example Rawtherapee) and assembly 
time-lapse video by [command line tools](https://github.com/Karry/TimeLapse/).

## Basic features:

- capture photos by phone camera (Qt camera api) or external camera (Gphoto2 library)
  - external camera needs to support PTP over USB and connected via USB OTG adapter
  - list of supported cameras: http://www.gphoto.org/proj/libgphoto2/support.php
  - automatic and continuous shutter-speed tuning to avoid under/over exposure  
- preprocess captured series (with ImageMagick library)
  - "deflickering" by luminance average of all images or moving average
  - blend frames (when video frame count is bigger than picture count)
- assembly video from captured series (using ffmpeg)
  - support x264 or x265 video encoders 

## Examples

What is time-lapse video? Here are few examples...

All captured with Nikon D5100, raw processed by Rawtherapee, then assembled by TimeLapse tools.

[![Aurora behind clouds](https://img.youtube.com/vi/XsykUYhzCsE/0.jpg)](https://www.youtube.com/watch?v=XsykUYhzCsE)
[![Night timelapse](https://img.youtube.com/vi/mv7ci8BZZr8/0.jpg)](https://www.youtube.com/watch?v=mv7ci8BZZr8)
[![Grant Canyon sunset](https://img.youtube.com/vi/ugQ-LHx41fg/0.jpg)](https://www.youtube.com/watch?v=ugQ-LHx41fg)

