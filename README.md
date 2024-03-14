# Enlarger-LED-Lightsource
An Arduino based LED Lightsource project using the WS2812b LED matrix  

This projects build a LED light source for a photographic enlargers. 

All content is free to be used for non-commercial purposes.

Any LED power setting can be used from 16 LED (small 35mm direct light source), 64 LED (normal 35mm/6x6 light source)  to 256-512 or even more LED for LF enlargers (by changing the #define NUMPIXELS constant in the codce).
The base is the use of the WS2812b LED. Each LED has a power consumption of 60mA (0.06A) keep this in mind when selecting an appropriate 5V power source. 
The WS2812b LED has following wavelengths: Red: 620-625nm / Green 522-525nm / Blue 465-467nm.

Most small enlargers have a light bulb that can be replaced with a smaller LED matrix. Some other enlargers can be adapted by bypassing the “old” tungsten light source with a new diffusion box that has an LED matrix included.

For the calculation of the needed power of the WS2812b modules, you may consider this:

Amount of LED	Size of module	Power consumption at 5V	Current	Equivalent Halogen
16 LED:	3x3 to 4x4 cm,	4-5W, 0.9A power supply needed,	25W tungesten equivalent
64 LED: 	8x8 to 10x10 cm,	19-21W, 3.5A power supply needed,	75-100W tungesten equivalent
256 LED: 	16x16 to 17x17 cm,	75-80W,	15.5A power supply needed,	250-300W tungesten equivalent

I am always cautious with this “equivalent” transformation, as the LED will not give all the power if modulated in the color needed. The amount of effective light can be up to 50% less. However, this is also the case if you use filters on a color head.

There are several other form factors of WS2812 LED matrixes or similar. There are round and circular, hexagonal and other shapes. Consider that a single WS2812 LED gives 0.25-0.3W of power and needs 0.06A of current. 

The PDF Documentation has a partslist. Most of the material may be found for a convenient price at Aliexpress or eBay. Local online shops have these components too, but my experience is the parts are the same.

Marco Glattfelder / CH-8856 Tuggen / marco.glattfelder@alphaobjects.ch 
