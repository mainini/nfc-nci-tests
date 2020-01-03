Little playground for [PN7120](http://www.nxp.com/products/identification-and-security/nfc-and-reader-ics/nfc-controller-solutions/full-nfc-forum-compliant-controller-with-integrated-firmware-and-nci-interface:PN7120A0EV)-NFC-controller and [linux_libnfc-nci](https://github.com/NXPNFCLinux/linux_libnfc-nci).

* *versions.c* : try to fetch FW- and MW-version from chip
* *poll.c* : first try at discovering tags
* *poll_mt* : multithreaded version with proper handling of incomming tag_info

Don't expect anything in here to actually work, do something useful or be of any further help...
