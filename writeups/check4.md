Checkpoint 4 Writeup
====================

My name: [Yousef AbuHashem]

My SUNet ID: [yousef24]

I collaborated with: [jtrb]

I would like to thank/reward these classmates for their help: [flynnd, gdavid, wbshelu]

This checkpoint took me about [5] hours to do. I [did] attend the lab session.

Program Structure and Design of the NetworkInterface:

[   The program has one main map that contains IP to Ethernet mappings, in addition to ARP requests. The way we could distinguish between those is wheter the Ethernet address has value associated to it.

    The other states my program keeps track of are a variable that keeps getting incremented when tick is called, and a queue that holds the ethernetframes that are ready to be sent.

    Everytime tick is called, the entire map is looped over to remove expired mappings. This is computaionally expensive. There are two other approaches I was thinking about:

    1 - A lazy approach where I only check if a mapping is expired when there is a need to send frames through it, if so, I delete it and send an ARP request. This is lazy computation, but holds mappings for longer.

    2 - An implementation where we only loop over the map elements that are expired. This could be achieved by having some datatype (like a priority queue) that orders the mappings based on their expiry date, and everytime tick is called, we delete mappings up to a certain point. 

    I was planning to do approach number 2, but I ended up sticking with the approach I did first which is computationally more expensive. I just had a bunch of projects to work on, and did not have the time to revisit this implementation!

    Minor note: I created multiple helper function to make my code look a little nicer and more readable
]

Implementation Challenges:
[   Generally speaking, once I had a good idea on the structure of the program and the differnt data structures I am using, I did not have many problems in the implementation.
One thing that took me some time was trying to decompose my code and write helper functions, basically minimizing the amount of repetitive code when I make frames for example.

One thing that also took some time was to get used to each data type and how to use different functions and methods that are already implemented. With some digging, it was fine.]

Remaining Bugs:
[None! I hope(?)]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [How drawing a roadmap and a general structure of the program before starting was almost all of the brain work I had to do!]

- Optional: I'm not sure about: [describe]
