# CAPTCHA

CAPTCHA is a C program that generates captcha images.


# Features

- TTF support
- Font size
- Pixel omission
- Lens distortion


# Usage

./captcha [ttf path] [challenge]


## Required arguments

- **ttf path**: path to true-type font
- **challenge**: CAPTCHA text to generate


## Optional arguments

- **-s**: font size
- **-k**: lens distortion parameter


## Output

- **captcha.bmp**


# Build

Replace **EXT_INCLUDE** & **EXT_LIBRARY** in the Makefile to include the following dependencies.


## Dependencies

- [FreeType](https://freetype.org/)  

