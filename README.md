# Flight Radar

![Flight Radar Image](https://media.harryskerritt.co.uk/assets/flight-radar.png)

**I love being able to see what's flying about in the sky, so made a way to visual it!**

This is a Flight Radar programmed in C using the Raylib Library!


### How it works
This project utilises the [Open Sky API](https://opensky-network.org/data/api) to pull current airspace data through the use of the [libcurl](https://github.com/curl/curl) library.
This is then parsed using the [cJson](https://github.com/davegamble/cjson) library and fed into [raylib](https://github.com/raysan5/raylib) to visualise it on the screen!

There is a MacOS app bundle available!
