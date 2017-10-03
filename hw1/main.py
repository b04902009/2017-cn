import socket
import time

IRC = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
IRC.connect(('irc.freenode.net', 6667))

def send(command):
	IRC.send('PRIVMSG '+ CHAN + ' :' + command + '\n')
def login(nickname, username, password='test', realname='ROBOT', servername='ROBOT', hostname='ROBOT'):
	IRC.send("USER %s %s %s %s\n" % (username, hostname, servername, realname))
	IRC.send("NICK %s\n" % nickname)
def is_valid_address(address):
	try:
		socket.inet_pton(socket.AF_INET, address)
	except socket.error:  # not a valid address
		return False
	if address.count('.') != 3:
		return False
	for seg in address.split('.'):
		if len(seg) > 3:
			return False
		if len(seg) > 1 and seg[0] == '0':
			return False
	return True

def insert_dot(string, index):
    return string[:index] + '.' + string[index:]

f = open('config', 'r')
CHAN = f.read()[6:-1]
login('b04902009', 'eiffel')
IRC.send("JOIN %s\n" % CHAN)
send('Hello! I am robot.')

def func(msg):
	# Nickname in used
	if msg.startswith('Nickname is already in use'):
		IRC.send("NICK %s\n" % str(time.time()))
		IRC.send("JOIN %s\n" % CHAN)	
	# @repeat
	elif msg.startswith('@repeat'):
		send(msg[8:])
	# @convert
	elif msg.startswith('@convert'):
		msg = msg[9:]
		try:
			if msg[:2] == '0x':
				send(str(int(msg, 16)))
			else:
				send(str(hex(int(msg))))
		except ValueError:
			send('Invalid number!')
	# @ip
	elif msg.startswith('@ip'):
		msg = msg[4:]
		length = len(msg)
		if length < 4:
			send('0')
		else:
			s = []
			for i in range(1,4):
				new = insert_dot(msg, i)
				for j in range(i+2,i+5):
					if j > length:
						break
					new1 = insert_dot(new, j)
					for k in range(j+2,j+5):
						if k > length+1:
							break
						new2 = insert_dot(new1, k)
						if is_valid_address(new2) == True:
							s.append(new2)
			
			send(str(len(s)))
			for i in s:
				send(i)
				time.sleep(1)
	# @help
	elif msg.startswith('@help'):
		send('@repeat <Message>')
		send('@convert <Number>')
		send('@ip <String>')
	# @quit
	elif msg.startswith('@quit'):
		IRC.send('quit\r\n')
		return False
	return True

while True:
	end = False
	rec = IRC.recv(4096)
	print rec
	for msg in rec.split('\r\n'):
		# PING PONG
		if msg[0:4] == 'PING':
			IRC.send('PONG '+ msg.split()[1] + '\r\n')
		elif msg.find('PRIVMSG'):
			msg = msg[msg.find(':', 1)+1:]
			if not func(msg):
				end = True
				break
	if end:
		break
IRC.close()