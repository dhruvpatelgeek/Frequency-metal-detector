
Frequency metal detector with discrimination
-------------------------------------------------
![alt text](https://github.com/dhruvpatelgeek/Frequency_metal_detector/blob/master/photos/coil.jpg)

VIDEO LINK
https://vimeo.com/405417001


1> Microcontroller  System: pic32 
2> Sensor Coil: built my own inductor which runs at 45Khz 
3> Colpitts  Oscillator: the system has a Colpitts  Oscillator 
4> C  programming: the system is programmed in C 
5> Speaker: the system does have a speaker 
6> Detection: can detect all Canadian coins 
7> up to 10 cm detection mode with discrimination (Fe, Al, Cu, Ni)
8> precision detection mode for small objects (coins, ball bearings)
9> sound based discrimination mode (beep if the type of metal is correct)
10> LEDs (RRYYB) to tell proximity (red = far.... blue = close)
11> speaker changes pitch with distance
12> automatic calibration for different environments




INDEX
-------------------------------------------------

0> Parts list
    
        0.1>list of all the components you need to get started

1> Hardware
   
    
        1.1> Working + construction of the Colpitts  Oscillator
        1.2> Working + construction of the Metal discrimination circuit 
        1.3> how to assemble the circuit

2> Software
   
    
         2.1> compilation/building of software
         2.2> how to flash it on the board


3> References
   
         3.1> online links+textbook References


 MAIN DOCUMENT            
 ------------------------------------------------- 

0> Parts list
   
        0.1>list of all the components you need to get started

        0.1.1> MCU circuit 
        2x red LED
        2x yellow LED 
        1x blue LED
        1x white LED
        5x green LED
        4x 5.1k ohm resistor 
        2x 3.9k ohm resistor 
        3x push-buttons
        1x buzzer
        1x pn2222A transistor
        4x 1000uF capacitor
        2x 1uF capacitor
        1x 100uF capacitor
        1x BOX32
        1x PIC32MX130F064B MCU
        1x MCP1700

        0.1.2> Colpitts oscillator 
        1x 104G-mosfet
        1x 2955G-mosfet
        2x 0.33uF capacitor
        1x 1k resistor
        1.5 meter copper wire

        0.1.3> sensor coil
        0.6 meter copper wire 
        40 ohm resistor
        27 ohm resistor
        1uF capacitor
        2x 10uF capacitor
        2x 1N007 diode
        1x Lm339N – comparator
        1x Op07cp – op amp
        1x LMC7660

1> Hardware
![alt text](https://github.com/dhruvpatelgeek/Frequency_metal_detector/blob/master/photos/DSC00034.JPG)

    1.1> Working + construction of the Colpitts  Oscillator

        1.1.1> The coil
        Simply take 1.5 meters of the copper wire (has to be insulated).
        and wrap it around a plastic or cardboard container leave 20cm slack to allow moment and connection to the circuit.

        refer 
        photos/coil.jpg

        1.1.2> The Oscillator
        The Colpitts  Oscillator is made up of two discrete MOSFETs 
        (104G) and (2955G) and in a CMOS inverter configuration
        one end of the Oscillator's coil is connected to the input 
        of this NOT gate via a 1K resistor and the other end is connected 
        to the output both the ends of the coil are connected
        to 0.33uF capacitors which are connected to ground.

        refer 
        photos/1.1 schematic of the oscillator.jpg
        photos/Colpitts  Oscillator.jpg

        It is essential that the PMOS is connected to a Voltage source of atleast 5Vs to make sure the Oscillator can start. Then Simply
        connect the output of the coil to LM339 comparator with the -ve of
        comparator connected to ground and the pull of the comparator
        connected to 1k resistor to 3.3V(for pic 32). Now connect the 
        output of the comparator to PIN RB13 of the pic32 MCU 
        and a diode for undervolt protection.

    1.2> Working + construction of the Metal discrimination circuit

        1.2.1> The Receiver Coil
        Just like in part 1.1.1 take 0.6 meters of wire and wrap it around as   shown int the photo here. Make sure to stick it with some tape so 
        that the coil doesn't change its diameter(VERY IMPORTANT).
        refer 
        photos/coil.jpg
        
        1.2.2> The arrangement of the two coils
        Connect one end of the coil to a 10uF capacitor to get LC oscillations.
        as shown here
        photos/coil.jpg
        Now probe one end of this coil with an oscilloscope and set the scale to 20mV.
        turn on the Colpitts oscillator that you made in part 1.1 and bring this coil close the the Colpitts's coil.

        VERY CAREFULLY place the coil below the Colpitts's coil and tape both of them
        with some electric tape in thin configuration
        photos/rx_coil.jpg
        Now slightly nudge the Receiver coil such that you don't see any waveforms greater
        than 5mv. Now both the coils are in equilibrium.

        1.2.2> Reading the signal from the receiver coil

            1.2.2.1>Low-pass-filter 
            Now connect the output of the Receiver coil to a low pass filter made of
            67ohm resistor and a 1uF capacitor to filter out the high frequency noise 
            that could mess with the ZERO-cross detection circuit.

            refer 
            photos/Rx_scheme.jpg

            1.2.2.2>OP-AMP to amplify the input
            Now connect output of the LPF to an op07cp OPAMP in non- inverting amplifier 
            configuration to get an amplification of about 1000x, as shown in the picture
            above.

            1.2.2.3>ZERO Cross detection.
            Now Simply connect the output of the op amp to the LM339 comparator with the 
            -ve of the comparator to gnd and +ve to a 1k pull up resistor to 3.3Vs.Connect this 
            output to pin RB5
        
    1.3> how to assemble the circuit
            1.3.1>Power supply
            connect lead 2 and 1 of the MCP1700 to a 1uf capacitors and the other capacitor lead
            to ground. Connect lead 2 to 5V powerline and Lead 3 to a new powerline. this would 
            now be used to power the pic_32 Microcontroller
            refer photos/pic32.jpg
            refer LED.png 

            1.3.2>Pic32 MCU
            Make connections with BOX230XS as shown in the figure below
            refer photos/pic32.jpg
            and
            refer LED.png

            1.3.3>speaker
            refer LED.png 
            Connect pin rb14 to the pn2222A transistor's base, wit the emitter connected to 
            gnd and the collector connect to one end of the speaker. Connect The other end 
            of the speaker to 3.3V powerline.
            
            1.3.4>LED System
            refer LED.png 
            Connect the 4 green LEDs cathode to a 5.1K ohm resistor and then to pins 
                RB15  //al          
                RB6   //fe         
                RA2   //cu         
                RA3   //ni 
            Connect the 2 red LEDs cathode to a 4.7K ohm resistor and then to pins 
                RA0   //very low signal
                RA1   //low signal
            Connect the 2 yellow LEDs cathode directly to pins 
                RB0   //medium signal
                RB1   //medium high signal
            Connect the Blue LED cathode to a 3.9K ohm resistor and then to pin
                RB3   //high signal
            Connect the white LED cathode to a 3.9K ohm resistor and then to pin
                RB2   // mode indication      
            make sure resistor values are correct so that LEDs glow equally bright.
            (normally red would be very bright and blue would be super dim).
                     

2> Software
    
    2.1> compilation/building of software
        2.1.1>period detection
            A simple implementation set RB5 as digital I/O pin
            and feed the oscillator out this this pin. Wait 
            for the signal to go high, start measuring
            and then wait for the single to go low then stop measuring.
            then 
                    f=1/Time measured;

        2.1.2>phase detection
            Set RB13 as digital I/O then feed the RX signal to this port.
            since the phase is always going to be 0-180 degrees in this  as we don't 
            need to account fo -ve phase. Wait for the Colpitts signal to go 0
            start counting. Then wait for the RX signal to go 1. 
            The measured value is a linear function of phase.

            WARNING: i don't measure the 'phase'(in deg that is) but a linear function of phase.
                     because i don't find the need to waste clock cycles to compute 
                     it since, the auto-calibration feature works just fine with this 'phase'

        2.1.3>variable sound pitch 
            assign a global variable pwm and change the pwm from 0 to 50 to change
            the pulse width of the square wave that runs the speaker
        2.1.4>LEDs
            set all the LED bits are User Programmable by writing to the 
                    TRISBbits register array.
            then use get the bit register to 0 to activate the LEDs by sinking current.

        2.1.5>I/O
            set port RB12 to digital I/O then

            while ((PORTB&(1<<12)?1:0)==0)
            {
                //block condition
            }
            // unblock condition
            

        2.1.6>different modes
            declare a variable flag such that it is incremented 
            when you push the button at RB12
             
             then have an if ladder to mimic a FSM

             if(state 1)
             else if(state 2)
             ...
             ...
             else
                default

    2.2> how to flash it on the board
            2.2.1> the software uses makefile the main program is call blinky.c
            >>>>>>go to crosside and open the make file 
            >>>>>>click clean
            >>>>>>click compile all
            >>>>>>connect usb to BOX230XS
            >>>>>>click LoadFlash
            >>>>>>all LEDs would light up indicating a successful flash 


3> References
    
    concepts from>>
    Purcell electricity and magnetism
    H.C verma part 2
    NCERT physics book 2
    Fundamentals of Electric Circuits by Charles K. Alexander and Matthew N.O. Sadiku

    the design form>>
    https://www.youtube.com/watch?v=fnwgf5RrhTg&list=LLD_NovgZm-r7sagGtF1yOAw&index=5&t=0s

    technical reference>>

    pic32 datasheet
    http://ww1.microchip.com/downloads/en/DeviceDoc/PIC32MX1XX2XX%20283644-PIN_Datasheet_DS60001168L.pdf
    op07cp
    http://www.ti.com/lit/ds/symlink/op07c.pdf
    Lm339N
    http://www.ti.com/lit/ds/symlink/lm339-n.pdf



