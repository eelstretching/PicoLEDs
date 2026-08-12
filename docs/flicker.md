I'm trying to diagnose a problem where the LEDs on some strips or panels flicker when I'm running with parallel LED rendering turned on. This has been bugging me for more than a year.

I'm reasonably certain that it's not an electrical problem. Most of the time when diagnosing flickering LEDs the problem will often be that the strip is not sharing the same ground as the data source, but I don't believe that to be the problem here.

I also have level-shifters between the Pico's data pins and the input on the strips, as suggested by various online sources. I do not have a resistor on the data line, however.

If I isolate the panel that is flickering in a parallel rendering scenario (say, by using the StripTest program and telling it that I only want to light up that panel), then everything works just perfectly. I can even set up a parallel renderer on four panels including the panels that flicker (and nothing else) and it runs just fine. 

Here's another example: using the StripTest program with a grid of 16 panels (treating them just like strips), I can tell it that the first strip is on pin 10 and that there are 8 strips. This generates a Renderer with one parallel PIO program (from ws2812.pio) that sends the data to the strips in parallel. This works fine.

If I tell the StripTest program that the first strip is on pin 2 and there are 16 strips, it generates a Renderer with two parallel PIO programs, one for the first 8 strips and one for the second 8. In this case, panels 15 and 16 flicker like crazy!

Given that the panels work in some instances and not others, I don't think it's a pure electrical problem.

I do note that the flickering tends to happen on the last couple of panels or strips in a parallel run.

I would say it's a timing problem for the LEDs, but the timing for the LEDs is the same in all the cases, including rendering a single panel.

I've looked at a bunch of different parallel rendering PIO programs and they all seem to have this problem to a greater or lesser degree.

Can you have a look at the c++ code and the PIO code to see if you can figure out whether there is some software reason why I might be experiencing this flickering? 
