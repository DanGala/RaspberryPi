# -*- coding: utf-8 -*-
"""
Created on Wed Mar 14 17:31:36 2018

@author: ffm
"""
from transitions.extensions import GraphMachine as Machine
#from transitions import Machine
import random

class ActivityDetector(object):

    states = ['init', 'silence', 'low', 'high', 'activity', 'final']

    def __init__(self, name):

        self.name = name

        # Current frame energy
        self.energy = 0
        
        # Count of silent frames
        self.silent_frames = 0

        # Count of activity frames
        self.activity_frames = 0

        # Energy thresholds
        self.low_factor = 3
        self.high_factor = 9
        self.final_factor = 6
        
        # Temporal thresholds
        # Minimum number of consecutive silent frames (energy below final_factor) to identify the end of an activity
        self.sil_req_frames = 2
        # Minimum length (in frames) of any activity
        self.act_req_frames = 4
        
        
        # Initialize the state machine
        self.machine = Machine(model=self, states=ActivityDetector.states, initial='init')
        
        # in cases where auto transitions should be visible
        # Machine(model=m, show_auto_transitions=True, ...)

        # Add some transitions!!!
        self.machine.add_transition(trigger='start', source='init', dest='silence')
	self.machine.add_transition('fire', 'silence', 'high', after='activity_function', conditions=['high_factor_cond'])
	self.machine.add_transition('fire', 'silence', 'low', conditions=['low_factor_cond'])
	self.machine.add_transition('fire','silence', 'silence')
	self.machine.add_transition('fire', 'low', 'high', after='activity_function', conditions=['high_factor_cond'])
	self.machine.add_transition('fire', 'low', 'silence', conditions=['silence_factor'])
	self.machine.add_transition('fire', 'low', 'low')
	self.machine.add_transition('fire', 'high', 'final', after='silence_activity', conditions=['final_factor_cond'])
	self.machine.add_transition('fire', 'high', 'high', after='activity_function')
	self.machine.add_transition('fire', 'final', 'high', after='activity_function', conditions=['final_high_factor'])
	self.machine.add_transition('fire', 'final', 'activity', after='reset', conditions=['is_voice'])
	self.machine.add_transition('fire', 'final', 'silence', after='reset', conditions=['is_silence'])
	self.machine.add_transition('fire', 'final', 'final', after='silence_activity')
	self.machine.add_transition('fire', 'activity', 'high', after='activity_function', conditions=['high_factor_cond'])
	self.machine.add_transition('fire', 'activity', 'low', conditions=['low_factor_cond'])
	self.machine.add_transition('fire', 'activity', 'silence')
        

    # Add your condition and callback methods
    def say_my_name(self):
        print(self.name)

    def activity_function(self):
	self.activity_frames += 1

    def silence_activity(self):
	self.silent_frames += 1

    def reset(self):
	self.activity_frames = 0
	self.silent_frames = 0
    
    def high_factor_cond(self):
	return self.energy > self.high_factor
    
    def low_factor_cond(self):
	return (self.energy > self.low_factor) and (self.energy < self.high_factor)
    
    def silence_factor(self):
	return self.energy < self.low_factor
    
    def final_factor_cond(self):
	return self.energy < self.high_factor
    
    def is_voice(self):
	return (self.silent_frames >= self.sil_req_frames) and (self.activity_frames >= self.act_req_frames)
    
    def is_silence(self):
	return (self.silent_frames >= self.sil_req_frames) and (self.activity_frames < self.act_req_frames)
    
    def final_high_factor(self):
	return (self.silent_frames < self.sil_req_frames) and (self.energy >= self.high_factor)
                
AD = ActivityDetector("heisenberg")

# draw the whole graph ...
AD.machine.get_graph().draw('Activity-Detector.png', prog='dot') 

# ... or just the region of interest
# (previous state, active state and all reachable states)
AD.machine.get_graph(show_roi=True).draw('Activity-Detector-ROI1.png', prog='dot') 

signal_energy=[0,0,4,8,10,11,12,10,5,5,0,0]

print AD.state
AD.start()
print AD.state

for x in range(0, len(signal_energy)):
    AD.energy = signal_energy[x]
    AD.fire()
    print "energy(%d) = %d - STATE = %s - activity_frames = %d - silent_frames = %d" % (x, signal_energy[x], AD.state, AD.activity_frames, AD.silent_frames)

exit()
