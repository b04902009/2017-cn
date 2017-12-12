make:
	gcc sender.c -o sender
	gcc agent.c -o agent
	gcc receiver.c -o receiver
# run:
# 	./sender 127.0.0.1 5001 file.txt
# 	./agent 127.0.0.1 5000 127.0.0.1 5002 0.2
# 	./receiver 127.0.0.1 5001 output.txt
clean:
	rm -r sender agent receiver