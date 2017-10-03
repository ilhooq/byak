#!/usr/bin/python

import os
import pexpect
import re
import sys

basePath = os.path.realpath(os.path.dirname(os.path.dirname(__file__)))

# Default epd file to read
epdFile  = basePath + "/tests/perft.epd"

if len(sys.argv) > 1:
	if sys.argv.count('-h'):
		print "Usage : perft.py [<epdFile>]\nDescription: Test Byak perft against a list of fen"
		sys.exit()

	if os.path.isfile(sys.argv[1]):
		epdFile = sys.argv[1]
	else:
		print "%s doesn't exsist or cannot be read" %  sys.argv[1]
		sys.exit()


with open(epdFile, 'r') as fp:
	lines = fp.read().split("\n")

child = pexpect.spawn(basePath + "/build/byak")
child.logfile = sys.stdout

child.sendline('uci');
child.expect('uciok');

for line in lines:
	# Empty line
	if len(line) == 0:
		continue
	# Comentary
	if line[:1] == '#':
		continue

	m = re.match(r"^(.*) [0-9]+ [0-9]+$", line)
	fen = m.group(1)

	m = re.search(r"([0-9]+) ([0-9]+)$", line)
	depth =  m.group(1)
	nodes =  m.group(2)

	print "\nEPD line -> fen: %s Depth:%s nodes:%s" % (fen, depth, nodes)

	child.sendline("position fen %s" % fen);
	child.expect("");
	child.sendline("perft %s tt" % depth);
	child.expect("nodes:%s" % nodes, timeout=60);







