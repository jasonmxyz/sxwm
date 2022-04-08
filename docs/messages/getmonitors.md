# `SXWM_GETMONITORS` message description

This file describes the function and structure of messages with type
`SXWM_GETMONITORS`.

The `SXWM_GETMONITORS` message type is used to request information about the
current screen configuration in SXWM. A client can determine the number of
physical displays, and for each of them can tell their names according to
XRandR, their size and position on the default display, a unique identifier,
and a list of workspaces contained on the monitor.

The header file `<sxwm/msg.h>` includes structure definitions useful for
encoding and decoding these messages, and the function `SXWMGetMonitors`
defined defined in `<sxwm.h>` can be used to get the current screen
configuration in a more accessible format.

# Request Format

SXWM will detect and respond to messages with the type `SXWM_GETMONITORS`, this
message type does not require any additional data to be sent in the request. It
is acceptable to send only the generic message header with no data by setting
`sxwm_header.size = 0`. If any data is sent with the message, SXWM will discard
it.

# Response Format

SXWM will send a response with type `SXWM_GETMONITORS` and include the same
sequence number as the initially received message.

The response data contains a number of sections:

1. __Header__ (Fixed Size): This contains the data required to determine the
   size of the remaining sections.
2. __Monitor Data__ (Variable Size): This section contains an array of
   structures with information about the monitors in use by SXWM.
3. __Workspace IDs__ (Variable Size): This section contains an array of
   integers representing identifiers of workspaces contained within the
   monitors.
4. __Monitor Names__ (Variable Size): This section contains null terminated
   strings representing the names of the monitors according to XRandR.

## Header

This section can be interpreted as a `struct sxwm_message_getmonitors_header`:
```c
struct sxwm_message_getmonitors_header {
	uint32_t nMonitors;
	uint32_t monitorDataOffset;
	uint32_t workspaceIDsOffset;
	uint32_t namesOffset;
};
```

- The `nMonitors` variable contains the number of items stored in the 'Monitor
  Data' section.
- `monitorDataOffset` stores the position of the start of the 'Monitor Data'
  section relative to the start of the message in bytes.
- `workspaceIDsOffset` stores the position of the start of the 'Workspace IDs'
  section relative to the start of the message in bytes.
- `namesOffset` stores the position of the start of the 'Monitor Names' section
  relative to the start of the message in bytes.

## Monitor Data

This section can be interpreted as an array of `struct
sxwm_message_getmonitors_monitordata`:

```c
struct sxwm_message_getmonitors_monitordata {
	uint32_t id;
	uint32_t nameOffset;
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
	uint32_t nWorkspaces;
	uint32_t selectedWorkspace;
	uint32_t workspacesOffset;
};
```

The number of the above structures in the array is determined by the value of
`nMonitors` in the 'Header' section.

- `id` stores the unique identifier of this monitor. It can be sent with later
  messages to identify this monitor.
- `nameOffset` stores the position of the start of the null terminated string
  representing the name of this monitor relative to the start of the 'Monitor
  Names' section in bytes.
- `x` is the x-position of the monitor on the default screen.
- `y` is the y-position of the monitor on the default screen.
- `width` is the width of the monitor on the default screen.
- `height` is the height of the monitor on the default screen.
- `nWorkspaces` is the number of workspaces contained on this monitor.
- `selectedWorkspace` is the unique identifier of the workspace which is
  currently selected on this monitor.
- `workspaceOffset` stores the index of the first workspace in the monitor in
  the `uint32_t` array in the 'Workspace IDs'.

## Workspace IDs

This section could be considered an array of `uint32_t` values. Each of them
represents the unique identifier of a workspace connected to a monitor. It is
possible to define a reference to the array like this:

```c
uint32_t *workspaces = (uint32_t *)(data + workspaceIDsOffset);
```

Then select the workspaces on a particular monitor like this:

```c
uint32_t *monitorWorkspaces = &(workspaces[monitorData.workspacesOffset]);
for (int i = 0; i < monitorData.nWorkspaces; i++) {
	uint32_t workspaceID = monitorWorkspaces[i];
	// Do something...
}
```

## Monitor Names

This section contains null terminated strings. We could determine the name of
monitor with code like this:

```c
char *names = (char*)(data + namesOffset);
```
```c
char *monitorName = &(names[monitorData.nameOffset]);
```