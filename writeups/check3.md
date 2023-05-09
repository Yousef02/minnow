Checkpoint 3 Writeup
====================

My name: [Yousef AbuHashem]

My SUNet ID: [yousef24]

I collaborated with: [jtrb]

I would like to thank/reward these classmates for their help: [drewkaul]

This checkpoint took me about [20] hours to do. I [did] attend the lab session.

Program Structure and Design of the TCPSender:
[ 
    General program structure: I have two maps, one for pushed and one for outstanding messages (after maybe_send is called on them). The (key, value) pairs are absolute seqnos and TCP messages.
    
    Starting with my push, push tries to put as much as possible in each message including SYN and FYN.
    For that, I enter a while loop that keeps pushing messages while size is allowing and while we have date to send. inside each loop, I keep peeking from the bytestream and make each message as big as possible.

    Retransmission: in my implementation, I just add messages again to the push map, I do not remove them from the outstanding list to make it easier to reset the timer when I recieve acks

    ]

Implementation Challenges:
[ I think there were places where the spec could have been more obvious ]

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
