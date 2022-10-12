#!/usr/bin/env python

from __future__ import division
import os, random, subprocess, time, threading, sys, pygame
from os import listdir
import os.path
from subprocess import PIPE
from time import sleep
import RPi.GPIO as GPIO
import Adafruit_PCA9685


GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)

servos = Adafruit_PCA9685.PCA9685()
rightLid_close = 500  # Min pulse length out of 4096
rightLid_open = 200  # Max pulse length out of 4096
leftLid_close = 200  # Min pulse length out of 4096
leftLid_open = 500  # Max pulse length out of 4096
pupil_close = 550  # Max pulse length out of 4096
pupil_open = 300  # Min pulse length out of 4096

pygame.init()
SET_VOLUME = 100
GUEST_PRESENT = False
SCALE_DEFAULT = 32768
AIR_SOLENOID = 17
RED_LIGHT = 18
PRESSURE_PLATE = 24
BUTTON = 23
PIR = 25
FOG_MACHINE = 16

INPUT1 = False
INPUT2 = False
INPUT3 = False

GPIO.setup(AIR_SOLENOID, GPIO.OUT)
GPIO.setup(FOG_MACHINE, GPIO.OUT)
GPIO.setup(RED_LIGHT, GPIO.OUT)
GPIO.setup(PIR, GPIO.IN)
GPIO.setup(BUTTON, GPIO.IN)
GPIO.setup(PRESSURE_PLATE, GPIO.IN)



#set system volume
set_vol_cmd = 'sudo amixer cset numid=1 -- {volume}% > /dev/null' .format(volume = SET_VOLUME)
os.system(set_vol_cmd)  # set volume

# def set_servo_pulse(channel, pulse):
    # pulse_length = 1000000    # 1,000,000 us per second
    # pulse_length //= 60       # 60 Hz
    # print('{0}us per period'.format(pulse_length))
    # pulse_length //= 4096     # 12 bits of resolution
    # print('{0}us per bit'.format(pulse_length))
    # pulse *= 1000
    # pulse //= pulse_length
    # servos.set_pwm(0, 0, pulse)
    # servos.set_pwm(1, 0, pulse)
    # servos.set_pwm(2, 0, pulse)
    # time.sleep (.3)
    # servos.set_pwm_freq(60)
    # time.sleep (1)
    
def playmusic(aud_type):
    action = 'pygame'

    #setting the correct directory for the wavs
    working_dir = '/home/pi/' + aud_type + '/'

    #creating array of wav files from the directory
    aud_files = [ f for f in listdir(working_dir) if f[-4:] == '.wav' ]

    #Make sure that there are actually wav files
    if not (len(aud_files) > 0):
        print "No wav files found!"

    #creating a random selection for which wav to play and setting that song to its own variable
    index = random.randint(0, len(aud_files)-1)
    song_name = working_dir + aud_files[index]
    if os.path.isfile(song_name):
        current_song = pygame.mixer.Sound(song_name)
    else:
        print 'the wrong trousers, grommet!'
        
    #debugging stuff
    print 'action = ' + action
    print 'index = ' + str(index)
    print 'wav file = ' + aud_files[index]
    print 'current song = ' + song_name

    if os.path.isfile(song_name):
        current_song.play()
        print '--- Playing ' + aud_files[index] + ' ---'
        time.sleep (.3)
    else:
        print 'the wrong trousers, grommet!'


def eyes_open():
    GPIO.output(RED_LIGHT, True)
    time.sleep(.3)    
    #start turning servo
    servos.set_pwm(0, 0, rightLid_open)
    time.sleep(.2)
    servos.set_pwm(1, 0, leftLid_open)
    time.sleep(.7)
    servos.set_pwm(2, 0, pupil_open)
    time.sleep(.4)



def eyes_close():
    #start turning servo
    GPIO.output(RED_LIGHT, False)
    time.sleep(.3)
    servos.set_pwm(0, 0, rightLid_close)
    time.sleep(.5)
    servos.set_pwm(1, 0, leftLid_close)
    time.sleep(.5)
    servos.set_pwm(2, 0, pupil_close)

    
def smoke_mouth_closed():
    GPIO.output(FOG_MACHINE, True)
    time.sleep(1.5)
    GPIO.output(FOG_MACHINE, False)
    time.sleep(1)
    GPIO.output(FOG_MACHINE, True)
    time.sleep(1.5)
    GPIO.output(FOG_MACHINE, False)
    time.sleep(.3)
    

def smoke_mouth_open():
    GPIO.output(FOG_MACHINE, True)
    time.sleep(.2)


def chomp():
    oneChomp(2,1)
    oneChomp(1,.5)
    oneChomp(.5,.3)
    oneChomp(.5,.3)
    oneChomp(1.5,.3)
    oneChomp(.5,.5)
    oneChomp(.5,.3)
    oneChomp(.5,1)
    oneChomp(.5,.5)
    oneChomp(.5,.3)
    oneChomp(.5,1)

def oneChomp (oTime, cTime):
    GPIO.output(AIR_SOLENOID, True)
    time.sleep(oTime)
    GPIO.output(AIR_SOLENOID, False)
    time.sleep(cTime)

def getInput():
    while True:
        if GPIO.input(BUTTON) == GPIO.HIGH:
            print 'Input received on Button'
            return True
        elif GPIO.input(PRESSURE_PLATE) == GPIO.HIGH:
            print 'Input received on Pressure Plate'
            return True
        elif GPIO.input(PIR) == GPIO.HIGH:
            print 'Input received on PIR'
            return True
        #else:
            #sleep(0.1)


def functioncheck():
    time.sleep(10)
    servos.set_pwm(0, 0, rightLid_open)
    time.sleep(1)
    servos.set_pwm(0, 0, rightLid_close)
    time.sleep(1)
    servos.set_pwm(1, 0, leftLid_open)
    time.sleep(1)
    servos.set_pwm(1, 0, leftLid_close)
    time.sleep(1)
    servos.set_pwm(2, 0, pupil_open)
    time.sleep(1)
    servos.set_pwm(2, 0, pupil_close)
    time.sleep(1)
    GPIO.output(RED_LIGHT, True)
    time.sleep(1)
    GPIO.output(RED_LIGHT, False)
    time.sleep(1)
    GPIO.output(AIR_SOLENOID, True)
    time.sleep(1)
    GPIO.output(AIR_SOLENOID, False)
    time.sleep(1)
    GPIO.output(FOG_MACHINE, True)
    time.sleep(1)
    GPIO.output(FOG_MACHINE, False)
    time.sleep(1)


def closeout():
    GPIO.output(FOG_MACHINE, False)
    GPIO.output(AIR_SOLENOID, False)
    GPIO.output(RED_LIGHT, False)
    eyes_close()

if __name__ == '__main__':

    servos.set_pwm_freq(60)
    functioncheck()
    time.sleep(.3)

    try:
        print 'Press Ctrl-C to quit.'

        while True:
            #getInput()
                        
            plate_is_on = GPIO.input(PRESSURE_PLATE)
            button_is_on = GPIO.input(BUTTON)
            pir_is_on = GPIO.input(PIR)

            if plate_is_on != INPUT1 or button_is_on != INPUT2 or pir_is_on != INPUT3:
                print 'The current state of the input pins is: Pressure Plate: ', plate_is_on, ', Button: ', button_is_on, ', PIR: ', pir_is_on
                INPUT1 = plate_is_on
                INPUT2 = button_is_on
                INPUT3 = pir_is_on

            if GPIO.input(PRESSURE_PLATE) == GPIO.HIGH or GPIO.input(BUTTON) == GPIO.HIGH or GPIO.input(PIR) == GPIO.HIGH:
                
                subprocess.call(['killall', 'pygame'])
                eyes_open()
                playmusic('SFX')
                smoke_mouth_closed()
                smoke_mouth_open()
                chomp()
                closeout()
            else:
                sleep(0.1)
    finally:
        subprocess.call(['killall', 'pygame'])
        servos.set_pwm_freq(60)
        GPIO.cleanup()
