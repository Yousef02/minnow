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

    Retransmission: in my implementation, I take messages out of the outstandingMap and I put them back into the map of pushed messages in order for them to be sent. Since maps are ordered, they will be given priority in sending when maybe_send is called.

    My timer mechanism is composed of a boolean to track whether it is on, rto, total time elapsed since a message was sent, and consecutive retransmitions for that message.

    my TCPsender also keeps track of the last ackno it got, which makes it easy to calculate what is in flight, it is just seqno_ (I keep track of which seqno should be assigned for the next push) - last ackno

    ]

Implementation Challenges:
[ I think there were places where the spec could have been more obvious. For example, I thought I could send the FIN in a seperate message, but then I realized I have to attach it to the payload if possible.

figuring out the nuances of when and how to reset and manipulate the timer was challenging, espicially that there are multiple places where differnt operations need to be done (I would have made a class if I had more time) 

Deciding on the size of the window, and when it should be set to zero was also challenging 

I kept hitting an infinite loop in my push, which was solved by a simple check if the TCP messsage was empty]

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [giving more detailed instructions on what the expectations are for different cases, maybe in the FAQ section for example]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [if my solution is actually good for a TCP or if it is just passing tests]
