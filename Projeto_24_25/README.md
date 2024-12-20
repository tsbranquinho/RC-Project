# Projeto RC - Group 67

This project implements a **client-server application** with various commands for managing and interacting with a game system (Mastermind) over a network. \
Tiago Branquinho - ist1106635 \
Rafael Avelar - ist1106546

---

## **Setup and Execution**

### **Build Instructions**
1. **Clean previous builds**  
   ```bash
   make clean
   ```

2. **Compile the project**  
   ```bash
   make
   ```

---

### **Running the Server**

#### **Run Options**
1. Run directly:  
   ```bash
   ./GS
   ```

2. Run with arguments:  
   ```bash
   ./GS -p port_no -v
   ```

3. Use the server directory:  
   ```bash
   ./server/GS
   ./server/GS -p port_no -v
   ```

4. Using `make`:  
   - Default server run:  
     ```bash
     make run-server
     ```
   - Run with arguments:  
     ```bash
     make run-server "ARGS=-p port_no -v"
     ```

#### **Note**  
- Running the server with `./GS` will create files and directories in the **main directory**.  
- Running `./server/GS` or `make` will create files and directories in the **child directory**.

---

### **Running the Client**

#### **Run Options**
1. Run directly:  
   ```bash
   ./player
   ```

2. Run with arguments:  
   ```bash
   ./player -n IP_address -p port_no
   ```

3. Use the client directory:  
   ```bash
   ./client/player
   ./client/player -n IP_address -p port_no
   ```

4. Using `make`:  
   - Default client run:  
     ```bash
     make run-client
     ```
   - Run with arguments:  
     ```bash
     make run-client "ARGS=-n IP_address -p port_no"
     ```

#### **Note**  
- Running the client with `./player` will create files and directories in the **main directory**.  
- Running `./client/player` or `make` will create files and directories in the **child directory**.

---

### **Tejo Network Details**
- **IP Address**: `193.136.138.142`  
- **Port Number**: `58011`

---

## **Client Commands**

The following commands are implemented in the client:

| **Command** | **Description**                   | **Status**  |
|-------------|-----------------------------------|-------------|
| `start`     | Start a new game session          | ✅ Completed |
| `try`       | Submit a guess                    | ✅ Completed |
| `quit`      | Quit the current game session     | ✅ Completed |
| `exit`      | Exit the client application       | ✅ Completed |
| `debug`     | Start a game session in debug mode| ✅ Completed |
| `trials`    | Fetch the trial history           | ✅ Completed |
| `scoreboard`| View the game scoreboard          | ✅ Completed |
| **Extras**  |                                   |             |
| `set`       | Set the player ID (takes `plid`)  | ✅ Completed |
| `clean`     | Clear .txt files                  | ✅ Completed |
| `sleep`     | Pause for a specified time (s)    | ✅ Completed |
| `help`      | Get the list of commands          | ✅ Completed |
| `hint`      | Get a hint for the current game   | ✅ Completed |

---

## **Server Commands**

The following commands are implemented on the server:

| **Command** | **Description**                   | **Status**  |
|-------------|-----------------------------------|-------------|
| `start`     | Start a new game session          | ✅ Completed |
| `try`       | Process a player's guess          | ✅ Completed |
| `quit`      | End a game session                | ✅ Completed |
| `debug`     | Start a game session in debug mode| ✅ Completed |
| `trials`    | Send trial history to the client  | ✅ Completed |
| `scoreboard`| Provide the game scoreboard       | ✅ Completed |
| **Extras**  |                                   |             |
| `hint`      | Provide a hint to the client      | ✅ Completed |

---