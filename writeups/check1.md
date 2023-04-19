Checkpoint 1 Writeup
====================

My name: [Yousef AbuHashem]

My SUNet ID: [yousef24]

I collaborated with: [jtrb]

I would like to thank/reward these classmates for their help: [ruiying]

This lab took me about [15] hours to do. I [did] attend the lab session.

Program Structure and Design of the Reassembler:
[My reassembler sits on two main ideas:
    - representing the data as a custom struct of first_index and the data itself
    - using a set (ordered by nature in C++) to be the reassembler's storage
    
    As insertion requests are happenning, I resolve any overlapping before adding the new insertion request to my reassembler's storage by shaving off portions on the newly coming piece of date, or completely erasing pieces already in the set if they are completely within the new insertion request.

    That choice makes it faster than cutting the string into chars.
    ]


Implementation Challenges:
[It was absolutely frustrating to track down bugs and off by one errors when dealing with overlapping data.]

Remaining Bugs:
[Thankfully, all tests passed briefly before the deadline!]

- Optional: I had unexpected difficulty with: [shaving off overlapping regions]

- Optional: I think you could make this lab better by: [giving more guidance! A TA discouraged the way I was approaching the problem, but did not really give any simpler approach.]

- Optional: I was surprised by: [Set operations and methods in c++]

- Optional: I'm not sure about: [if I still cover every single edge case!]
