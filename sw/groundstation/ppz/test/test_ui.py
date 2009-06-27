import os.path
import time
import gtk
import unittest

import ppz.ui.treeview as treeview
import ppz.ui.senders as senders

#common constants to test against
from testcommon import *

# Stolen from Kiwi
def refresh_gui(delay=0):
	while gtk.events_pending():
		gtk.main_iteration_do(block=False)
	time.sleep(delay)

class TreeviewTest(unittest.TestCase):
    def setUp(self):
        self.mf = get_mf()

    def testConstruct(self):
        ts = treeview.MessageTreeStore()
        for m in self.mf.get_messages():
            ts.add_message(m)
        tv = treeview.MessageTreeView(ts)

        refresh_gui()

class RequestMessageSenderTest(unittest.TestCase):
    def setUp(self):
        self.mf = get_mf()
        self.rm = senders.RequestMessageSender(self.mf)

    def testSend(self):
        #yuck, fake pass by reference
        sent = [None, None]
        def send_message(btn, msg, vals, _sent):
            _sent[0] = msg
            _sent[1] = vals

        self.rm.connect("send-message", send_message, sent)
        refresh_gui()
        self.rm._cb.set_active(0)
        refresh_gui()
        self.rm._btn.clicked()
        refresh_gui()

        sendmsg = sent[0]
        requestmsg = self.mf.get_message_by_name( "REQUEST_MESSAGE" )
        vals = sent[1]

        self.failUnlessEqual(sendmsg.id, requestmsg.id)
        self.failUnless( len(vals), 1 )

class SimpleMessageSenderTest(unittest.TestCase):
    def setUp(self):
        self.mf = get_mf()
        self.sm = senders.SimpleMessageSender(self.mf)

    def testSend(self):
        #yuck, fake pass by reference
        sent = [False]
        def send_message(btn, msg, vals, _sent):
            _sent[0] = True

        self.sm.connect("send-message", send_message, sent)
        refresh_gui()
        self.sm._cb.set_active(0)
        refresh_gui()
        self.sm._btn.clicked()
        refresh_gui()

        self.failUnless(sent[0])

if __name__ == "__main__":
    unittest.main()
