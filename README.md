# bplist_parser
This is an exention of plistutils in libplist 

When the plist is NSKeyArchived one, you would be very hard to understand the structure and check the data

Now, just as you use plistutils, bplist_parser will handle this kind plist for you.

Currently, it suoport
NSDictionary
NSArray
NSDate
NSNumber
NSString

If there are other objects archived by custom classes they will be ignored
