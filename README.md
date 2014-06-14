Spark-Core-IRremote
===================
By: qwertzguy

A modified version of [Arduino-IRremote](https://github.com/shirriff/Arduino-IRremote) for the Spark Core v1.0 and compatible with the Spark Core web IDE.

Modifications
-------------

* Trimmed out IRrecv (removed the decoding of IR signals. Thus you can only send IR codes with this lib. If you make the IRrecv part of the code copatible, please send a pull-request :) )
* Integrated the Spark Core's PWM. The PWM is set to 50% during mark and 0% during space. 
* The carrier frequency is in KHz, but due to the way the PWM works can be up tp 500 Hz off at IR frequencies
* Allows to set the pin of the IR led - no checking is done to ensure that PWM is permitted on that pin.
* Allows to make multiple instance of the IRsend class with different pins to control multiple IR leds.
* Updated to place IR codes in flash instead of RAM arrays

Usage examples
--------------

**WARNING**: The library has the same usage as the original IRremote, except that the constructor takes the pin as first argument: `IRsend(int irPin);`. For example, if you want to instanciate the class (on the stack) for the pin D1 use: `IRsend irsend(D1);`.

You can look at usages examples for sending on the original library for various TVs (make sure you make the change in the above warning for the examples to work): [https://github.com/shirriff/Arduino-IRremote/tree/master/examples](https://github.com/shirriff/Arduino-IRremote/tree/master/examples)

I used it to control my Roomba using this gist (make sure you make the change in the above warning for the examples to work): [https://gist.github.com/probonopd/5181021](https://gist.github.com/probonopd/5181021)

Help
----

If you don't understand what this library does, how to use it, what is a Spark Core or what is infrared remote codes, look at this wiki [help page](https://github.com/qwertzguy/Spark-Core-IRremote/wiki/Help).
