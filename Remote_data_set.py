import RPi.GPIO as GPIO
import smbus
from time import sleep

GPIO.setmode(GPIO.BCM) 
slaveAddress = 0x22  
messageInterruptPIN = 26
GPIO.setup(messageInterruptPIN, GPIO.IN)
bus = smbus.SMBus(1)
GPIO.add_event_detect(messageInterruptPIN, GPIO.RISING)


def make_raw(fin):
    raw = []
    for j in range (80):
        intval = fin[j][0]*256 + fin[j][1]
        if(intval == 255):
            print (raw)
            return raw
        else:
            raw.append(intval)
            
def readMessageFromArduino():
    int_message = []
    for i in range (80): 
        int_message.append(bus.read_i2c_block_data(slaveAddress,0,2))

    RAW=make_raw(int_message)
    print(make_pronto(RAW))
    return RAW


def int_to_hex(num):
    temp_hex=hex(num)
    if (len(str(temp_hex[2:]))==4):
        return temp_hex[2:]
    elif (len(str(temp_hex[2:]))==3):
        return '0'+ temp_hex[2:]
    elif (len(str(temp_hex[2:]))==2):
        return '00'+ temp_hex[2:]
    elif (len(str(temp_hex[2:]))==1):
        return '000'+ temp_hex[2:]


def make_pronto(raw_code):
    pronto_hex=[]
    frequency = 38000
    time_period =(1/frequency)*1000000
    burst_pairs_count = int(len(raw_code)/2 + 1)
    pronto_hex.append('0000')
    freq = int(1000000/(frequency*0.241246))
    pronto_hex.append(int_to_hex(freq))
    pronto_hex.append('0000')
    pronto_hex.append(int_to_hex(burst_pairs_count))
    for c in raw_code:
        pronto_hex.append(int_to_hex(int(c/time_period)))
    pronto_hex.append('06ae')
    return pronto_hex


def run():
    state = int(input("Enter 1 or 0 to send the command to arduino :   "))
    if(state==1):
        bus.write_byte(slaveAddress,0x1)
#          bus.write_i2c_block_data(slaveAddress,0x00,string_to_bytes("s"))
    state = 0    
    
    while 1:
        try:
            if GPIO.event_detected(messageInterruptPIN):
                print("detected")
                try:
                    mess = readMessageFromArduino()
                    break
                except IOError:
                    pass
        except KeyboardInterrupt:
            GPIO.cleanup()
    return mess  

def compare(message1,message2):
    if (len(message1) != len(message2)):
        print("Lengths are not the same !!!!")
        return False
    else:
        for i in range(len(message1)):
            if (message2[i] < (message1[i]+(15/100)*(message1[i])) and message2[i] > (message1[i]-(15/100)*(message1[i]))):
                return True
            else:
                return False

            
def check():
    c=0
    i=0
    while i<3:
        print("want to get data? for the " + str(i+1) + "st time of 3 times?")
        if(i==0):
            code = run()
            i=i+1
            final_code = code 
            print("1111")
        else:
            print("1112")
            code2=run()
            if(compare(code2,code)):
                print("1113")
                i=i+1
                final_code = code
            else:
                print("1114")
                c=c+1
                if(c<3):
                    print("1115")
                    print("codes are  not identical. Try again!!")
                    i=0
                    final_code = []
                else:
                    print("1116")
                    print("too many attempts!!!!")
                    final_code = []
                    break
    return final_code


def send_to_arduino(final_message,repeat_value,pin_number):
    while True:
        try:
            write_data(final_message,repeat_value,pin_number)
            break
        except IOError:
            pass
       
    
def make_pronto_string(pro):
    pronto_string=""
    for i in pro:
        pronto_string += i
    return pronto_string        

def string_to_bytes(val):
    retVal=[]
    for c in val:
        retVal.append(ord(c))
    return retVal


def write_data(final_message,repeat_value,pin_number):
    code = make_pronto_string(final_message)
    code += str(repeat_value)
    code += str(pin_number)
    start_index=0
    section = code[start_index:start_index+4]
    while (start_index < len(code)):
        byte_value = string_to_bytes(section)
        bus.write_i2c_block_data(slaveAddress,0x00,byte_value)
        start_index += 4
        section = code[start_index:start_index+4]  
    return -1


Arduino_message = make_pronto(run())
user_message = input("enter your pronto hex code for test : ")
user_message_array = user_message.split(" ")

#check()
print("which code do you want to send to Arduino?")
print("1.input code \n2.Arduino code \n0.None of them")
state = int(input("Enter 1 or 2 or 0 : "))

if(state==1):
    repeat_value = int(input("enter the repeat value : "))
    pin_number = int(input("enter the pin number : "))
    send_to_arduino(user_message_array,repeat_value,pin_number)
    print("SENT")
elif(state==2):
    repeat_value = int(input("enter the repeat value : "))
    pin_number = int(input("enter the pin number : "))
    send_to_arduino(Arduino_message,repeat_value,pin_number)
    print("SENT")
else:
    pass

#print("this is the final code for the button  " + str(make_pronto(check())))