# ceiling-fan

## Ceiling Fan control from deCONZ
Copyright © 2018 Erik Baauw. All rights reserved.

This repository documents my efforts for controlling my dumb ceiling fans from [deCONZ](https://github.com/dresden-elektronik/deconz-rest-plugin), and, through [homebridge](https://github.com/nfarina/homebridge) and [homebridge-hue](https://github.com/ebaauw/homebridge-hue), from HomeKit.
As no-one seems to have done this before (see [deCONZ issue #717](https://github.com/dresden-elektronik/deconz-rest-plugin/issues/717)), I decided to try and hack together my own solution.

I have three regular ceiling fans.  Each fan has a single-phase 240V, 50Hz AC motor, and three GU.10 light spots.  As I've already installed smart lights, I'm only looking for a solution to smartify the control the fan speed, and possibly the direction.

The fan has four connection wires under the canopy: Motor Life (black), Lights Life (also black), Neutral (blue) and Ground (green/yellow).  My ceiling outlet only has three wires: Life (black, from the traditional wall switch) Neutral (blue), Earth (green/yellow), so the motor and lights are controlled by a single wall switch wire.  The smart lights prefer to be powered on always, so the wall switch is used only to cut power from the fan, not for regular control.  As I don't want to pull a new switched life wire, replacing the wall switch is not an option.

### ZigBee in-wall switch
My first attempt was to try and install a ZigBee in-wall switch in the fan's canopy, in between the incoming life wire and the motor life wire.  I mutilated an Xiaomi Aqara in-wall switch (`lumi.ctrl_ln2`), took off the front and cut off the edges supporting the keys, leaving a 6cm by 6cm by 3cm box, which fits the canopy.  After assembling the fan, it worked.  No speed control, but deCONZ was able to switch the fan on and off at speed preset by the pull-chain switch.

However, the Xiaomi switch broke while I was balancing the fan's blades (frequently switching on and off the fan using the Xiaomi switch).  The relays switch off automatically a second after switching on, or won't switch on at all.  Not sure if this is just my luck, or whether the switch doesn't like the fan starting up.  The fan is 50W, while the switch is rated for 2500W (10A).  Incidentally, one of my OSRAM Lightify smart plugs (the old model) exhibits the same symptoms.  It was powering a portable fan...

Before the switch broke, I already ordered two more Xiaomi switches, which just arrived.  Only these are the newer ones without a Neutral connection (they use a minute current on the one of the L1 or L2 wires to power the electronics).  Time permitting, I'll try and install these to see if they won't break as well.

### Speed Control
Even if the new switches won't break, it would be so much cooler also to be able to control the fan speed from HomeKit.  The in-wall smart dimmers I've seen are rated for lights only (and probably beyond my budget anyways).

With each pull on the chain, the fan speed cycles through _Off_, _High_, _Medium_, _Low_.  The pull-chain switch has a simple mechanical mechanism, that switches it's input to one of three output wires (presumably connected to different coil windings on the AC motor).  So, at least in theory, fan speed could be controlled by three relays, when making sure at most one output line is powered at a time.  Doing that in software seem risky, given the beta status of deCONZ and the hobby status of homebridge-hue, so I'd prefer a hardware solution.

This hardware solution would be chaining the relays, connecting the NC (normally closed) output of one relay to the COM input of a second, and NO (normally open) output to the COM of a third.  This way, I can switch between four outputs: the second relay's NO and NC, and the third relay's NO and NC.  The second relay switches between _Off_ (NC) and _Low_ (NO), the third between _Medium_ (NC) and _High_ (NO), while the first relay switches between the second (NC) and third (NO) relays.

R1 | R2 | R3 | R2 NC | R2 NO | R3 NC | R3 NC | Speed
---| -- | -- | ----- | ----- | ----- | ----- | ----- | -----
off | off | off | power | | (no input) | | _Off_
off | off | on* | power | | | (no input) | _Off_
off | on | off | | power | (no input) | | _Low_
off | on | on* | | power | | (no input) | _Low_
on | off | off | (no input) | | power | | _Medium_
on | off | on | (no input) | | | power | _High_
on | on* | off | | (no input) | power | | _Medium_
on | on* | on | | (no input) | | power | _High_

\*) Since there's no power on the input, best leave the relay off.

While I would still occasionally need to switch two relays for a single transition (e.g. from _Off_ to _High_), only one output is connected to the life wire at all times.

### First Proof of Concept
Before messing with 240V, I figured I'd better create a low-voltage proof of concept.
I got myself an Arduino Relay Shield v3, which has four 10A, 240V relays.  This still leaves the fourth relay available for direction control.  Using that, I build the following prototype, using my Ardino Starter Kit: ![](Poc/PoC 1_bb.jpg)
