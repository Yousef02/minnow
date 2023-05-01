Checkpoint 2 Writeup
====================

My name: [Yousef AbuHashem]

My SUNet ID: [yousef24]

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: [atj10]

This lab took me about [12] hours to do. I [did] attend the lab session.

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:
[
    Wrap: a simple cast of n + the zero point to a uint32
    Unwrap: gets two values higher and lower than the checkpoint and checks which is closer. Throught the process, I make sure nothing overflows wiht numbers.


    TCPReciever: the recieve method unwraps the seqno of the message, converting it into a stram index (substracting 1 when needed since the SYN flag takes a place in seqo but not in stream indecies). The send method handles sending the correct achno numbers and window sizes based on what we have gotten so far + some adjustments for the SYN and FIN flags ( either adding 1 or 2 when wrapping the stram indices pushed )
]

Implementation Challenges:
[
    Unwrap: I have had challenges grasping how unwrapping happens at first. Tenchincally, I had issues with overflowing since all my data types are unsigned, so I had to make sure I have guards against that.

    In the recieve method, I took some time to realize when we have to substract 1 from the abs seqno before we get the stream index.
]

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]

- Optional: I made an extra test I think will be helpful in catching bugs: [describe where to find]
