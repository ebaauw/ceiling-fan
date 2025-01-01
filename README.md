# ceiling-fan

## Ceiling Fan control from deCONZ
Copyright © 2018-2025 Erik Baauw. All rights reserved.

This repository documents my efforts for controlling my dumb ceiling fans from [deCONZ](https://github.com/dresden-elektronik/deconz-rest-plugin), and, through [homebridge](https://github.com/nfarina/homebridge) and [homebridge-hue](https://github.com/ebaauw/homebridge-hue), from HomeKit.
As no-one seems to have done this before (see [deCONZ issue #717](https://github.com/dresden-elektronik/deconz-rest-plugin/issues/717)), I decided to try and hack together my own solution.

I have three regular ceiling fans.  Each fan has a single-phase 240V, 50Hz AC motor, and three GU.10 light spots.  As I've already installed smart lights, I'm only looking for a solution to smartify the control the fan speed, and possibly the direction.

The fan has four connection wires under the canopy: Motor Life (black), Lights Life (also black), Neutral (blue) and Ground (green/yellow).  My ceiling outlet only has three wires: Life (black, from the traditional wall switch) Neutral (blue), Earth (green/yellow), so the motor and lights are controlled by a single wall switch wire.  The smart lights prefer to be powered on always, so the wall switch is used only to cut power from the fan, not for regular control.  As I don't want to pull a new switched life wire, replacing the wall switch is not an option.

### ZigBee In-Wall Switch
My first attempt was to try and install a ZigBee in-wall switch in the fan's canopy, in between the incoming life wire and the motor life wire.  I mutilated an Xiaomi Aqara in-wall switch (`lumi.ctrl_ln2.aq1`), took off the front and cut off the edges supporting the keys, leaving a 6cm by 6cm by 3cm box, which fits the canopy.  After assembling the fan, it worked.  No speed control, but deCONZ was able to switch the fan on and off at speed preset by the pull-chain switch.

However, the Xiaomi switch broke while I was balancing the fan's blades (frequently switching on and off the fan using the Xiaomi switch).  The relays switch off automatically a second after switching on, or won't switch on at all.  Not sure if this is just my luck, or whether the switch doesn't like the fan starting up.  The fan is 50W, while the switch is rated for 2500W (10A).  Incidentally, one of my OSRAM Lightify smart plugs (the old model) exhibits the same symptoms.  It was powering a portable fan...

Before the switch broke, I already ordered two more Xiaomi switches.  They turned out to be the newer ones without a Neutral connection (they use a minute current on the one of the L1 or L2 wires to power the electronics).  That means they're powered off, when the pull-chain is in the off position.   Also, these are the end-device variants (`lumi.ctrl_neutral2`), now fully supported in deCONZ v2.05.37.  I mutilated the housing and installed them, fingers crossed.

#### Update after Several Months.
I got another `lumi.ctrl_ln2.aq1` switch for the third fan.  It worked for a couple of weeks, but now shows same defect as the first one.  It crashes every 60 to 70 seconds, sending a new device announcement and switching off the relays on subsequent startup.  The relays seem to work fine, the problem seems to be with the switch's power supply and/or firmware.  I haven't been able to isolate a particular ZigBee message that would cause the crash.  Also the crashes are too regular to be caused by irregularities in the mains power.  I double checked the first switch: it shows the same behaviour, but crashes even more frequently.

The two `lumi.ctrl_neutral2` switches still work.  I ordered a third one (I hope) to replace the broken switch.

### Speed Control
Even if the new switches won't break, it would be so much cooler also to be able to control the fan speed from HomeKit.  The in-wall smart dimmers I've seen are rated for lights only (and probably beyond my budget anyways).

With each pull on the chain, the fan speed cycles through _Off_, _High_, _Medium_, _Low_.  The pull-chain switch has a simple mechanical mechanism, that switches it's input to one of three output wires (presumably connected to different coil windings on the AC motor).  So, at least in theory, fan speed could be controlled by three switches, when making sure at most one output line is powered at a time.  Doing that in software seem risky, given the beta status of deCONZ and the hobby status of homebridge-hue, so I'd prefer a hardware solution.  Also, I haven't come across triple-gang in wall switches, so I'd need to use at least two.  No way two Xiaomi switches will fit, and two ubisys S2 switches per fan is beyond my budget.

The hardware solution would be based on chaining relays, connecting the NC (normally closed) output of one relay to the COM input of a second relay, and NO (normally open) output to the COM of a third relay.  This way, I can switch between four outputs: the second relay's NO and NC, and the third relay's NO and NC.  The second relay switches between _Off_ (NC) and _Low_ (NO), the third between _Medium_ (NC) and _High_ (NO), while the first relay switches between the second (NC) and third (NO) relays.

RL1 | RL2 | RL3 | NC 2 | NO 2 | NC 3 | NC 3 | Speed
----| --- | --- | ---- | ---- | ---- | ----- | -----
off | off | off | power | | (no input) | | _Off_
off | off | on* | power | | | (no input) | _Off_
off | on  | off | | power | (no input) | | _Low_
off | on  | on* | | power | | (no input) | _Low_
on  | off | off | (no input) | | power | | _Medium_
on  | off | on  | (no input) | | | power | _High_
on  | on* | off | | (no input) | power | | _Medium_
on  | on* | on  | | (no input) | | power | _High_

\*) Since there's no power on the input, best leave the relay off.

While I would still occasionally need to switch two relays for a single transition (e.g. from _Off_ to _High_), only one output is connected to the life wire at all times.

### First Proof of Concept
Before messing with 240V, I figured I'd better create a low-voltage proof of concept.
I got myself an [Ardino Starter Kit](https://www.arduino.cc/en/Main/ArduinoStarterKit) and a [Seeed Relay Shield v3](http://wiki.seeedstudio.com/Relay_Shield_v3/), with four relays.  This still leaves a fourth relay available for direction control.

To test the logic, I build the following prototype: ![image](https://raw.githubusercontent.com/ebaauw/ceiling-fan/main/PoC/PoC%201_bb.jpg)


While the relays themselves should be able to switch 240V AC up to 10A, I'm pretty sure that this will blow up the board (and probably the Arduino).  In this prototype I use the relays to switch 5V DC from the Arduino.  Not something you'd normally need a relay for, but fine for testing the logic.  Relay 1 is simulating the fan's direction, switching between _Down_ (NC) and _Up_ (NO, blue LED).  Relays 2 to 4 are used for the fan speed: relay 2 switches between relays 4 (NC) and 3 (NO); relay 4 switches between _Off_ (NC) and _Low_ (NO, green LED); relay 3 switches between _Medium_ (NC, yellow led) and _High_ (NO, red led).  The switches are used to simulate the pull-chain switches: S1 for pulling the speed switch and S2 for changing the direction switch.  I programmed the Arduino with the following [sketch](https://github.com/ebaauw/ceiling-fan/blob/master/PoC/PoC.ino).

This works as intended.  Next step will be figuring out the radio logic.

### MHCOZY 4-Channel Zigbee Relay
I finally found an AC-powered, 4-channel Zigbee relay with NO and NC outputs per channel: the [MHCOZY](https://www.amazon.nl/dp/B0C9Y5LTFJ/ref=pe_28126711_487102941_TE_SCE_dp_1).
According to its specs, the relays can be used to drive motors, so they should be happy with the fan's inductive load.
The downsides: it's quite large, 8 x 8 x 3cm, and it's Tuya.
