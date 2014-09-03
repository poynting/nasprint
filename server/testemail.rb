#!/usr/local/bin/ruby
# -*- encoding: iso-8859-1 -*-

require_relative 'email'

msg1 = OutgoingEmail.new
msg1.sendEmail("ns6t_ham@yahoo.com", "Testing 1.2.3.", "This is a test of the simple email interface.\n")
msg1 = nil

cab1 = "START-OF-LOG: 2.0
CREATED-BY: N3FJP's CA State QSO Party Program 2.2
CALLSIGN: CT2JBG
ARRL-SECTION: 
CONTEST: NCCC-CQP
CATEGORY: SINGLE-OP ALL QRP SSB
CLUB-NAME: 
CLAIMED-SCORE:  18 
OPERATORS: CT2JBG
NAME: Luis Ramos
ADDRESS: Rua Dr.Jos� Neves J�nior, Lote 13 - 1� Dt.
ADDRESS: 8000-332 Faro - Portugal
ADDRESS: (e-mail) ct2jbg@gmail.com
SOAPBOX: 
QSO: 21000 PH 2013-10-06 2029 CT2JBG     001 DX        K6TU      1099 SMAT
QSO: 21000 PH 2013-10-06 2034 CT2JBG     002 DX        W6YX       661 SCLA
QSO: 21000 PH 2013-10-06 2037 CT2JBG     003 DX        N60       1053 CCOS
END-OF-LOG:
"

cab2 = "START-OF-LOG: 3.0
CREATED-BY: DXLog.net v2.0.18
CONTEST: CA-QSO-PARTY
CALLSIGN: 9A1AA
CATEGORY-OPERATOR: SINGLE-OP
CATEGORY-TRANSMITTER: ONE
CATEGORY-ASSISTED: NON-ASSISTED
CATEGORY-OVERLAY: 
CATEGORY-POWER: HIGH
CATEGORY-BAND: ALL
CATEGORY-MODE: MIXED
CLAIMED-SCORE: 6336
CLUB: CROATIAN CONTEST CLUB
NAME: IVO NOVAK
ADDRESS: P.O.BOX 53
ADDRESS: HR 31551 BELISCE
ADDRESS: C R O A T I A
OPERATORS: 
SOAPBOX: 
X-CQP-EMAIL: ivo.9a1aa@gmail.com
X-CQP-CONFIRM1: ivo.9a1aa@gmail.com
QSO: 21313 PH  2013-10-05 1650 9A1AA         0001 DX   W6PZ          0103 SONO  
QSO: 21309 PH  2013-10-05 1651 9A1AA         0002 DX   K6Z           0092 INYO  
QSO: 21317 PH  2013-10-05 1652 9A1AA         0003 DX   WB6L          0081 VENT  
QSO: 21026 CW  2013-10-05 1656 9A1AA         0004 DX   K6AM          0066 SDIE  
QSO: 21031 CW  2013-10-05 1657 9A1AA         0005 DX   W6UE          0086 LANG  
QSO: 21035 CW  2013-10-05 1659 9A1AA         0006 DX   K6XX          0112 SCRU  
QSO: 21038 CW  2013-10-05 1700 9A1AA         0007 DX   K6VO          0090 SBER  
QSO: 21042 CW  2013-10-05 1702 9A1AA         0008 DX   W6PZ          0126 SONO  
QSO: 21046 CW  2013-10-05 1703 9A1AA         0009 DX   N6XI          0108 NEVA  
QSO: 21049 CW  2013-10-05 1704 9A1AA         0010 DX   N6RK          0040 SACR  
QSO: 21027 CW  2013-10-05 1719 9A1AA         0011 DX   AD6E          0098 TULA  
QSO: 21051 CW  2013-10-05 1721 9A1AA         0012 DX   N6NO          0060 SDIE  
QSO: 21031 CW  2013-10-06 1404 9A1AA         0013 DX   N6O           0648 CCOS  
QSO: 21040 CW  2013-10-06 1421 9A1AA         0014 DX   K6Q           1421 ELDO  
QSO: 21041 CW  2013-10-06 1423 9A1AA         0015 DX   K6WC          0241 SBEN  
QSO: 21042 CW  2013-10-06 1424 9A1AA         0016 DX   K6QK          1555 IMPE  
QSO: 21043 CW  2013-10-06 1425 9A1AA         0017 DX   K6LA          1380 LANG  
QSO: 21044 CW  2013-10-06 1429 9A1AA         0018 DX   K6MI          0374 TEHA  
QSO: 21026 CW  2013-10-06 1435 9A1AA         0019 DX   K6RIM         1102 MARN  
QSO: 21278 PH  2013-10-06 1440 9A1AA         0020 DX   KK6ZM         0752 SDIE  
QSO: 21284 PH  2013-10-06 1441 9A1AA         0021 DX   WC6H          1734 CALA  
QSO: 21321 PH  2013-10-06 1448 9A1AA         0022 DX   W6YX          0304 SCLA  
QSO: 21317 PH  2013-10-06 1454 9A1AA         0023 DX   K6XV          1011 SUTT  
QSO: 21042 CW  2013-10-06 1457 9A1AA         0024 DX   AE6Y          1591 AMAD  
QSO: 21034 CW  2013-10-06 1457 9A1AA         0025 DX   KG6N          0995 SHAS  
QSO: 21035 CW  2013-10-06 1459 9A1AA         0026 DX   NC6DX         0954 NEVA  
QSO: 21037 CW  2013-10-06 1500 9A1AA         0027 DX   N6YEU         0902 GLEN  
QSO: 21038 CW  2013-10-06 1501 9A1AA         0028 DX   KQ6ES         0408 SBER  
QSO: 21043 CW  2013-10-06 1503 9A1AA         0029 DX   WC6H          1769 CALA  
QSO: 21044 CW  2013-10-06 1503 9A1AA         0030 DX   AA6PW         0559 ORAN  
QSO: 21048 CW  2013-10-06 1508 9A1AA         0031 DX   W6AWW         0462 LANG  
QSO: 21048 CW  2013-10-06 1515 9A1AA         0032 DX   KJ6MBW        0334 ALAM  
QSO: 21055 CW  2013-10-06 1518 9A1AA         0033 DX   WN6K          0835 SDIE  
QSO: 21059 CW  2013-10-06 1521 9A1AA         0034 DX   N6MI          0683 KERN  
QSO: 28034 CW  2013-10-06 1537 9A1AA         0035 DX   N6O           0617 CCOS  
QSO: 28036 CW  2013-10-06 1539 9A1AA         0036 DX   K6QK          1658 IMPE  
QSO: 28037 CW  2013-10-06 1539 9A1AA         0037 DX   K6TK          1475 SLUI  
QSO: 28038 CW  2013-10-06 1540 9A1AA         0038 DX   W6SX          1348 MONO  
QSO: 28039 CW  2013-10-06 1540 9A1AA         0039 DX   K6RIM         1168 MARN  
QSO: 28050 CW  2013-10-06 1542 9A1AA         0040 DX   NN6CH         0732 ORAN  
QSO: 28042 CW  2013-10-06 1543 9A1AA         0041 DX   K6Z           0360 INYO  
QSO: 28037 CW  2013-10-06 1545 9A1AA         0042 DX   W6TK          1482 SLUI  
QSO: 28033 CW  2013-10-06 1546 9A1AA         0043 DX   W6UE          1584 LANG  
QSO: 28041 CW  2013-10-06 1550 9A1AA         0044 DX   AE6Y          1663 AMAD  
QSO: 28053 CW  2013-10-06 1551 9A1AA         0045 DX   N6GP          0072 ORAN  
QSO: 28029 CW  2013-10-06 1616 9A1AA         0046 DX   W6YX          0232 SCLA  
QSO: 21041 CW  2013-10-06 1626 9A1AA         0047 DX   W6TK          1527 SLUI  
QSO: 21045 CW  2013-10-06 1628 9A1AA         0048 DX   NN6CH         0763 ORAN  
QSO: 21047 CW  2013-10-06 1629 9A1AA         0049 DX   K6NV          0564 NEVA  
QSO: 21049 CW  2013-10-06 1630 9A1AA         0050 DX   N0DY/6        0641 ORAN  
QSO: 21051 CW  2013-10-06 1632 9A1AA         0051 DX   N6JV          0970 SACR  
QSO: 21053 CW  2013-10-06 1633 9A1AA         0052 DX   KA3DRR        0756 SLUI  
QSO: 21054 CW  2013-10-06 1634 9A1AA         0053 DX   K9JM          0676 NEVA  
QSO: 21055 CW  2013-10-06 1635 9A1AA         0054 DX   WB9JPS/6      0277 ALAM  
QSO: 21056 CW  2013-10-06 1636 9A1AA         0055 DX   W6XU          1022 SONO  
QSO: 21249 PH  2013-10-06 1638 9A1AA         0056 DX   KG6N          1064 SHAS  
QSO: 21256 PH  2013-10-06 1645 9A1AA         0057 DX   N6NF          1168 SMAT  
QSO: 21271 PH  2013-10-06 1647 9A1AA         0058 DX   N6O           0819 CCOS  
QSO: 21273 PH  2013-10-06 1648 9A1AA         0059 DX   K6VO          1399 SBER  
QSO: 21279 PH  2013-10-06 1649 9A1AA         0060 DX   W6T           2494 TRIN  
QSO: 21297 PH  2013-10-06 1654 9A1AA         0061 DX   W6TA          1376 LANG  
QSO: 21332 PH  2013-10-06 1700 9A1AA         0062 DX   K6MI          0522 TEHA  
QSO: 21339 PH  2013-10-06 1701 9A1AA         0063 DX   K6NA          1810 SDIE  
QSO: 21358 PH  2013-10-06 1704 9A1AA         0064 DX   K6LRN         1229 ELDO  
QSO: 21039 CW  2013-10-06 1713 9A1AA         0065 DX   N6M           0995 ALPI  
QSO: 21044 CW  2013-10-06 1724 9A1AA         0066 DX   N6FR          1589 ELDO  
QSO: 21048 CW  2013-10-06 1727 9A1AA         0067 DX   NX6T          1603 SDIE  
QSO: 21056 CW  2013-10-06 1728 9A1AA         0068 DX   K6LRN         1244 ELDO  
QSO: 21330 PH  2013-10-06 1732 9A1AA         0069 DX   W6AFA         0907 LANG  
QSO: 21357 PH  2013-10-06 1734 9A1AA         0070 DX   K6T           0418 TUOL  
QSO: 21376 PH  2013-10-06 1737 9A1AA         0071 DX   W6TK          1601 SLUI  
QSO: 21385 PH  2013-10-06 1739 9A1AA         0072 DX   NM6G          0470 MARP  
QSO: 21388 PH  2013-10-06 1740 9A1AA         0073 DX   W6FA          0753 NEVA  
END-OF-LOG:
"

msg2 = OutgoingEmail.new
msg2.sendEmail("ns6t_ham@yahoo.com", "Testing with attachments", "This is a test of the simple email interface.\n",
               [ { "mime" => "application/octet", "filename" => "CT2JBG.log", "content" => cab1 },
                 { "mime" => "application/octet", "filename" => "9A1AA.log", "content" => cab2 } ])
msg2 = nil
