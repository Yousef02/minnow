Checkpoint 5 Writeup
====================

My name: [Yousef AbuHashem]

My SUNet ID: [yousef24]

I collaborated with: []

I would like to thank/reward these classmates for their help: [joeytg, joonkeep, cmoffitt]

This checkpoint took me about [3] hours to do. I [did] attend the lab session.

Program Structure and Design of the Router:
[ When add_roure is called, it just collects all the parameters it got into a struct, and pushes it into a vector of routes that I made

    After that, in route, we check each interface from the given interfaces, while we can recieve datagrams from it, we check our rotes table if it has any destinantion that we can send it to.
    
    Cheking the longest compatable prefix in an address is done through shifting to the right by the length of the uint_32 - the length of the prefix; which leaves us with just the digits that the prefix specified.
    
    After that, if a route is found, we decrement its TTL and modify its cheksum and we send it to its next hop if it has one (its destination if it does not)]

Implementation Challenges:
[ Impleminting the router was pretty straightforward after I understood the different layers of the network which going to the lab wa very helpful for!

    I had an issue with looping by reference, I had some issues before I was doing that
    
    I also had an issue with shifting by 32 digits, which produces undefined behavior; I just dealt with that as a special case]

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
