# Centralized-Multi-User-Concurrent-Banking-System
# Centralized-Multi-User-Concurrent-Bank-Account-Manager- Designed a multithreaded system in C++ which executes transactions simultaneous from thousands of clients over TCP socket, applied distributed system concepts like processes, threads, concurrency control and synchronization mechanisms.

The project is implemented through socket communication to connect multiple clients with the server.  
- The server indefinitely calls accept system call to accept the client requests. 
- When a client connection is accepted, server creates a thread for that client. One thread is created for each client. 
- The thread created continuously reads on the client socket for upcoming data from the client.  
- Client after connecting with the server sends a transaction request (either a deposit / withdraw) to the server. 
- Server receives the request and performs validation checks like insufficient balance before withdrawing and after performing the action, sends the response back to the client. 
- All the transactions sent by the client and received at the server end are properly logged on server and client console. 
- Client waits after sending each transaction request for the time difference between two consecutive requests timestamp multiplied by the timestep. 
- Multiple clients when executed together may try to perform operation on the same account which may lead to inconsistent balance in the account. To avoid such situation, locking is applied on valid accounts.  
- Before any operation of withdraw or deposit is performed for any account, the section of critical code is locked for other transactions on the same account. Lock is released after completion of the operation, so that next operation on that account can start. - Two transactions on different accounts can happen simultaneously.  
- When client do not have any more data to send, the thread created at server for that client ends. - Server has a thread for calculating interest on each account periodically. The interest is calculated at the rate of .0001% every 10 minutes. Interest calculation is also protected by mutex lock. 

COMPILATION AND EXECUTION  
Following are the steps of execution through the makefile. 
- make clean //it cleans the .out file by executing rm -rf *.o compile 
- make compile //Complies the server and the client source codes creating the executable files server.out and client.out
- ./server <port> <path_to_records_file>  //Run server :
- ./client.out <machine_addr> <port> <timestep> <path_to_transaction_file>  //Run client
Format of the input files: Records.txt -> <account_number> <account_holder> <balance_in_account>  e.g., 101 Chang 10000 
Transaction.txt file format -> <timestamp> <account_number> <operation> <amount>  e.g., 5 101 d 1200 
