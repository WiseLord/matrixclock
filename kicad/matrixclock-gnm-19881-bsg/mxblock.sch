EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Driver_LED:MAX7219 U5
U 1 1 5FA7A7B0
P 3400 3950
AR Path="/5FA7A3F9/5FA7A7B0" Ref="U5"  Part="1" 
AR Path="/5FAA92F9/5FA7A7B0" Ref="U3"  Part="1" 
AR Path="/5FA7B411/5FA7A7B0" Ref="U?"  Part="1" 
AR Path="/5FA7B74C/5FA7A7B0" Ref="U?"  Part="1" 
AR Path="/5FA7BC34/5FA7A7B0" Ref="U6"  Part="1" 
AR Path="/5FA7BDAC/5FA7A7B0" Ref="U7"  Part="1" 
AR Path="/5FA7BF24/5FA7A7B0" Ref="U8"  Part="1" 
AR Path="/5FA7A7B0" Ref="U101"  Part="1" 
F 0 "U101" H 3150 4900 50  0000 C CNN
F 1 "MAX7219" H 3650 4900 50  0000 C CNN
F 2 "Package_SO:SOIC-24W_7.5x15.4mm_P1.27mm" H 3350 4000 50  0001 C CNN
F 3 "https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf" H 3450 3800 50  0001 C CNN
	1    3400 3950
	1    0    0    -1  
$EndComp
$Comp
L Device:R R7
U 1 1 5FA7A5A5
P 2800 2900
AR Path="/5FA7A3F9/5FA7A5A5" Ref="R7"  Part="1" 
AR Path="/5FAA92F9/5FA7A5A5" Ref="R2"  Part="1" 
AR Path="/5FA7B411/5FA7A5A5" Ref="R?"  Part="1" 
AR Path="/5FA7B74C/5FA7A5A5" Ref="R?"  Part="1" 
AR Path="/5FA7BC34/5FA7A5A5" Ref="R8"  Part="1" 
AR Path="/5FA7BDAC/5FA7A5A5" Ref="R9"  Part="1" 
AR Path="/5FA7BF24/5FA7A5A5" Ref="R10"  Part="1" 
AR Path="/5FA7A5A5" Ref="R101"  Part="1" 
F 0 "R101" H 2870 2946 50  0000 L CNN
F 1 "10k" H 2870 2855 50  0000 L CNN
F 2 "Resistor_SMD:R_1206_3216Metric_Pad1.30x1.75mm_HandSolder" V 2730 2900 50  0001 C CNN
F 3 "~" H 2800 2900 50  0001 C CNN
	1    2800 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	3000 3150 2800 3150
Wire Wire Line
	2800 3150 2800 3050
$Comp
L power:+5VD #PWR03
U 1 1 5FA7B94F
P 2800 2700
AR Path="/5FA7A3F9/5FA7B94F" Ref="#PWR03"  Part="1" 
AR Path="/5FAA92F9/5FA7B94F" Ref="#PWR06"  Part="1" 
AR Path="/5FA7B411/5FA7B94F" Ref="#PWR?"  Part="1" 
AR Path="/5FA7B74C/5FA7B94F" Ref="#PWR?"  Part="1" 
AR Path="/5FA7BC34/5FA7B94F" Ref="#PWR0123"  Part="1" 
AR Path="/5FA7BDAC/5FA7B94F" Ref="#PWR0128"  Part="1" 
AR Path="/5FA7BF24/5FA7B94F" Ref="#PWR0133"  Part="1" 
F 0 "#PWR0133" H 2800 2550 50  0001 C CNN
F 1 "+5VD" H 2815 2873 50  0000 C CNN
F 2 "" H 2800 2700 50  0001 C CNN
F 3 "" H 2800 2700 50  0001 C CNN
	1    2800 2700
	1    0    0    -1  
$EndComp
Wire Wire Line
	2800 2700 2800 2750
$Comp
L power:+5VD #PWR04
U 1 1 5FA7C761
P 3400 2700
AR Path="/5FA7A3F9/5FA7C761" Ref="#PWR04"  Part="1" 
AR Path="/5FAA92F9/5FA7C761" Ref="#PWR07"  Part="1" 
AR Path="/5FA7B411/5FA7C761" Ref="#PWR?"  Part="1" 
AR Path="/5FA7B74C/5FA7C761" Ref="#PWR?"  Part="1" 
AR Path="/5FA7BC34/5FA7C761" Ref="#PWR0124"  Part="1" 
AR Path="/5FA7BDAC/5FA7C761" Ref="#PWR0129"  Part="1" 
AR Path="/5FA7BF24/5FA7C761" Ref="#PWR0134"  Part="1" 
F 0 "#PWR0134" H 3400 2550 50  0001 C CNN
F 1 "+5VD" H 3415 2873 50  0000 C CNN
F 2 "" H 3400 2700 50  0001 C CNN
F 3 "" H 3400 2700 50  0001 C CNN
	1    3400 2700
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 2700 3400 2950
$Comp
L power:GNDD #PWR05
U 1 1 5FA7D156
P 3400 5050
AR Path="/5FA7A3F9/5FA7D156" Ref="#PWR05"  Part="1" 
AR Path="/5FAA92F9/5FA7D156" Ref="#PWR08"  Part="1" 
AR Path="/5FA7B411/5FA7D156" Ref="#PWR?"  Part="1" 
AR Path="/5FA7B74C/5FA7D156" Ref="#PWR?"  Part="1" 
AR Path="/5FA7BC34/5FA7D156" Ref="#PWR0125"  Part="1" 
AR Path="/5FA7BDAC/5FA7D156" Ref="#PWR0130"  Part="1" 
AR Path="/5FA7BF24/5FA7D156" Ref="#PWR0135"  Part="1" 
F 0 "#PWR0135" H 3400 4800 50  0001 C CNN
F 1 "GNDD" H 3404 4895 50  0000 C CNN
F 2 "" H 3400 5050 50  0001 C CNN
F 3 "" H 3400 5050 50  0001 C CNN
	1    3400 5050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 5050 3400 4950
$Comp
L matrixclock:GNM-19881-BSG U15
U 1 1 5FA7DCB0
P 5250 3500
AR Path="/5FA7A3F9/5FA7DCB0" Ref="U15"  Part="1" 
AR Path="/5FAA92F9/5FA7DCB0" Ref="U4"  Part="1" 
AR Path="/5FA7B411/5FA7DCB0" Ref="U?"  Part="1" 
AR Path="/5FA7B74C/5FA7DCB0" Ref="U?"  Part="1" 
AR Path="/5FA7BC34/5FA7DCB0" Ref="U16"  Part="1" 
AR Path="/5FA7BDAC/5FA7DCB0" Ref="U17"  Part="1" 
AR Path="/5FA7BF24/5FA7DCB0" Ref="U18"  Part="1" 
AR Path="/5FA7DCB0" Ref="U102"  Part="1" 
F 0 "U102" H 4600 4050 50  0000 L CNN
F 1 "GNM-19881-BSG" H 4950 3500 50  0000 L CNN
F 2 "matrixclock:GNM-19881-BSG" H 5150 2700 50  0001 C CNN
F 3 "http://www.kingbrightusa.com/images/catalog/SPEC/CA56-12EWA.pdf" H 5270 3530 50  0001 C CNN
	1    5250 3500
	1    0    0    -1  
$EndComp
Wire Wire Line
	3800 3150 4000 3150
Text Label 4000 3150 2    50   ~ 0
S1
Wire Wire Line
	3800 3250 4000 3250
Text Label 4000 3250 2    50   ~ 0
S2
Wire Wire Line
	3800 3350 4000 3350
Text Label 4000 3350 2    50   ~ 0
S3
Wire Wire Line
	3800 3450 4000 3450
Text Label 4000 3450 2    50   ~ 0
S4
Wire Wire Line
	3800 3550 4000 3550
Text Label 4000 3550 2    50   ~ 0
S5
Wire Wire Line
	3800 3650 4000 3650
Text Label 4000 3650 2    50   ~ 0
S6
Wire Wire Line
	3800 3750 4000 3750
Text Label 4000 3750 2    50   ~ 0
S7
Wire Wire Line
	3800 3850 4000 3850
Text Label 4000 3850 2    50   ~ 0
S0
Wire Wire Line
	3800 3950 4000 3950
Text Label 4000 3950 2    50   ~ 0
D0
Wire Wire Line
	3800 4050 4000 4050
Text Label 4000 4050 2    50   ~ 0
D1
Wire Wire Line
	3800 4150 4000 4150
Text Label 4000 4150 2    50   ~ 0
D2
Wire Wire Line
	3800 4250 4000 4250
Text Label 4000 4250 2    50   ~ 0
D3
Wire Wire Line
	3800 4350 4000 4350
Text Label 4000 4350 2    50   ~ 0
D4
Wire Wire Line
	3800 4450 4000 4450
Text Label 4000 4450 2    50   ~ 0
D5
Wire Wire Line
	3800 4550 4000 4550
Text Label 4000 4550 2    50   ~ 0
D6
Wire Wire Line
	3800 4650 4000 4650
Text Label 4000 4650 2    50   ~ 0
D7
Text HLabel 2600 4750 0    50   Input ~ 0
DIN
Wire Wire Line
	2600 4750 3000 4750
Text HLabel 2600 4650 0    50   Input ~ 0
CLK
Wire Wire Line
	2600 4650 3000 4650
Text HLabel 2600 4550 0    50   Input ~ 0
LOAD
Wire Wire Line
	2600 4550 3000 4550
Text HLabel 4650 4750 2    50   Output ~ 0
DOUT
Wire Wire Line
	4650 4750 3800 4750
Wire Wire Line
	4600 3150 4400 3150
Wire Wire Line
	4600 3250 4400 3250
Wire Wire Line
	4600 3350 4400 3350
Wire Wire Line
	4600 3450 4400 3450
Wire Wire Line
	4600 3550 4400 3550
Wire Wire Line
	4600 3650 4400 3650
Wire Wire Line
	4600 3750 4400 3750
Wire Wire Line
	4600 3850 4400 3850
Wire Wire Line
	4900 4300 4900 4100
Wire Wire Line
	5000 4300 5000 4100
Wire Wire Line
	5100 4300 5100 4100
Wire Wire Line
	5200 4300 5200 4100
Wire Wire Line
	5300 4300 5300 4100
Wire Wire Line
	5400 4300 5400 4100
Wire Wire Line
	5500 4300 5500 4100
Wire Wire Line
	5600 4300 5600 4100
Text Label 4900 4300 1    50   ~ 0
D0
Text Label 5300 4300 1    50   ~ 0
D4
Text Label 5500 4300 1    50   ~ 0
D6
Text Label 5100 4300 1    50   ~ 0
D2
Text Label 5200 4300 1    50   ~ 0
D3
Text Label 5600 4300 1    50   ~ 0
D7
Text Label 5400 4300 1    50   ~ 0
D5
Text Label 5000 4300 1    50   ~ 0
D1
Text Label 4400 3750 0    50   ~ 0
S1
Text Label 4400 3250 0    50   ~ 0
S6
Text Label 4400 3650 0    50   ~ 0
S2
Text Label 4400 3150 0    50   ~ 0
S7
Text Label 4400 3550 0    50   ~ 0
S3
Text Label 4400 3350 0    50   ~ 0
S5
Text Label 4400 3850 0    50   ~ 0
S0
Text Label 4400 3450 0    50   ~ 0
S4
$Comp
L Device:C C3
U 1 1 5FCC0C41
P 2650 3850
AR Path="/5FA7A3F9/5FCC0C41" Ref="C3"  Part="1" 
AR Path="/5FAA92F9/5FCC0C41" Ref="C2"  Part="1" 
AR Path="/5FA7B411/5FCC0C41" Ref="C?"  Part="1" 
AR Path="/5FA7B74C/5FCC0C41" Ref="C?"  Part="1" 
AR Path="/5FA7BC34/5FCC0C41" Ref="C4"  Part="1" 
AR Path="/5FA7BDAC/5FCC0C41" Ref="C5"  Part="1" 
AR Path="/5FA7BF24/5FCC0C41" Ref="C6"  Part="1" 
AR Path="/5FCC0C41" Ref="C101"  Part="1" 
F 0 "C101" H 2765 3896 50  0000 L CNN
F 1 "0.1" H 2765 3805 50  0000 L CNN
F 2 "Capacitor_SMD:C_1206_3216Metric_Pad1.33x1.80mm_HandSolder" H 2688 3700 50  0001 C CNN
F 3 "~" H 2650 3850 50  0001 C CNN
	1    2650 3850
	1    0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR02
U 1 1 5FCC0C42
P 2650 4050
AR Path="/5FA7A3F9/5FCC0C42" Ref="#PWR02"  Part="1" 
AR Path="/5FAA92F9/5FCC0C42" Ref="#PWR05"  Part="1" 
AR Path="/5FA7B411/5FCC0C42" Ref="#PWR?"  Part="1" 
AR Path="/5FA7B74C/5FCC0C42" Ref="#PWR?"  Part="1" 
AR Path="/5FA7BC34/5FCC0C42" Ref="#PWR0126"  Part="1" 
AR Path="/5FA7BDAC/5FCC0C42" Ref="#PWR0131"  Part="1" 
AR Path="/5FA7BF24/5FCC0C42" Ref="#PWR0136"  Part="1" 
F 0 "#PWR0136" H 2650 3800 50  0001 C CNN
F 1 "GNDD" H 2654 3895 50  0000 C CNN
F 2 "" H 2650 4050 50  0001 C CNN
F 3 "" H 2650 4050 50  0001 C CNN
	1    2650 4050
	1    0    0    -1  
$EndComp
$Comp
L power:+5VD #PWR01
U 1 1 5FAA2AB9
P 2650 3650
AR Path="/5FA7A3F9/5FAA2AB9" Ref="#PWR01"  Part="1" 
AR Path="/5FAA92F9/5FAA2AB9" Ref="#PWR04"  Part="1" 
AR Path="/5FA7B411/5FAA2AB9" Ref="#PWR?"  Part="1" 
AR Path="/5FA7B74C/5FAA2AB9" Ref="#PWR?"  Part="1" 
AR Path="/5FA7BC34/5FAA2AB9" Ref="#PWR0127"  Part="1" 
AR Path="/5FA7BDAC/5FAA2AB9" Ref="#PWR0132"  Part="1" 
AR Path="/5FA7BF24/5FAA2AB9" Ref="#PWR0137"  Part="1" 
F 0 "#PWR0137" H 2650 3500 50  0001 C CNN
F 1 "+5VD" H 2665 3823 50  0000 C CNN
F 2 "" H 2650 3650 50  0001 C CNN
F 3 "" H 2650 3650 50  0001 C CNN
	1    2650 3650
	1    0    0    -1  
$EndComp
Wire Wire Line
	2650 3650 2650 3700
Wire Wire Line
	2650 4000 2650 4050
NoConn ~ 4900 2800
NoConn ~ 5000 2800
NoConn ~ 5100 2800
NoConn ~ 5200 2800
NoConn ~ 5300 2800
NoConn ~ 5400 2800
NoConn ~ 5500 2800
Wire Wire Line
	4900 2800 4900 2900
Wire Wire Line
	5000 2800 5000 2900
Wire Wire Line
	5100 2800 5100 2900
Wire Wire Line
	5200 2800 5200 2900
Wire Wire Line
	5300 2800 5300 2900
Wire Wire Line
	5400 2800 5400 2900
Wire Wire Line
	5500 2800 5500 2900
Wire Wire Line
	5600 2800 5600 2900
NoConn ~ 5600 2800
$EndSCHEMATC
