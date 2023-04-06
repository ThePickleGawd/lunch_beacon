## What is this?

A "fastrak" for lunch lines aimed to get rid of lines. You put the device in your backpack, and then walk through the door without needing to scan or type your student id code.

## Notes

Add the following files to FIXME_EXAMPLE_USES_BLE in app.mk

```
FIXME_EXAMPLE_USES_BLE := \
 lunch_beacon.c \
 lunch_button.c \
 lunch_nvds.c \
 lunch_gatt.c \
```

## State Diagram

```mermaid
stateDiagram
    [*] --> S_INIT
    S_INIT --> S_IDLE: OP_MODULE_INIT
    S_IDLE --> S_STARTING_LUNCH_ADV: OP_CREATE_LUNCH_ADV
    S_INIT --> S_IDLE: OP_CREATE_PAIR_ADV
    S_IDLE --> S_STARTING_PAIR_ADV: OP_CREATE_PAIR_ADV
    S_STARTING_LUNCH_ADV --> S_ADV_STARTED: OP_CREATE_LUNCH_CFM
    S_STARTING_PAIR_ADV --> S_ADV_STARTED: OP_CREATE_PAIR_CFM
    S_ADV_STARTED --> S_IDLE: OP_CREATE_PAIR_ADV
    S_ADV_STARTED --> S_CONNECTED: OP_CONNECTED
    S_CONNECTED --> S_ADV_STOPPED: OP_DISCONNECTED
    S_CONNECTED --> S_CONNECTED: OP_ADV_TIMEOUT
    S_ADV_STARTED --> S_ADV_STOPPED: OP_ADV_TIMEOUT
    S_ADV_STOPPED --> S_IDLE: OP_DELETE_PAIR_ADV
    S_ADV_STOPPED --> S_IDLE: OP_SLEEP
```
