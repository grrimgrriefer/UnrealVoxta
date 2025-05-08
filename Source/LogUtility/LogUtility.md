# LogUtility Module

## Overview

LogUtility is a lightweight UE module for the Voxta plugin, focused on standardizing logging capabilities, particularly for handling sensitive information. Due to target audience for Voxta, most user-input or LLM generated output is not appropriate to be contained in analytics, logging, crashlogs, etc. This module streamlines the censorship of such info.

## Module Structure

The LogUtility module is designed to be minimal and self-contained, with dependencies only on the Core module of Unreal Engine. It consists of the following key files:

- `Public/Defines.h` - C++ logging macros

## Features

### Sensitive Data Logging

The primary feature of LogUtility is its sensitive data handling system that allows:

- Automatic censoring of sensitive information in logs (multi-parameter support)
- Easy toggle between showing and hiding sensitive data

### Configuration

Sensitive logging can be configured through the `SensitiveLogging` namespace:

```cpp
namespace SensitiveLogging
{
    static bool isSensitiveLogsCensored = true;
    static const FString CENSORED_TEXT_CONTENT = TEXT("****censored****");
}
```

- `isSensitiveLogsCensored` - Controls whether sensitive data is censored (default: true)
- `CENSORED_TEXT_CONTENT` - The text used to replace censored information

## Usage

The module provides several macros for logging sensitive information:

### Basic Sensitive Logging

Uses the old UE_LOG macro internally, good for support with pre-UE-5.2 logging.

```cpp
SENSITIVE_LOG_BASIC(LogCategory, Verbosity, Format, Param)
```

### Advanced Sensitive Logging (with formatting)

Uses the new UE_LOGFMT macro, supported from UE-5.2 and later.

For all SENSITIVE_LOG variants, if censoring is enabled, all parameters will be replaced with "****censored****":

```cpp
// Old-style UE_LOG format
SENSITIVE_LOG_BASIC(LogCategory, Verbosity, Format, Param)  

// New-style UE_LOGFMT format (UE 5.2)
SENSITIVE_LOG1(LogCategory, Verbosity, Format, Param1)      
SENSITIVE_LOG2(LogCategory, Verbosity, Format, Param1, Param2)
SENSITIVE_LOG3(LogCategory, Verbosity, Format, Param1, Param2, Param3)
```

## Example

```cpp
// Log with sensitive user data
FString Username = "JohnDoe";
FString Password = "SecretPassword123";

// This will output "User credentials: ****censored****, ****censored****" if censoring is enabled
SENSITIVE_LOG2(VoxtaLog, Warning, "User credentials: {0}, {1}", Username, Password);
```

## Dependencies

- UnrealEngine
  - `Core`

## Licensing

MIT license - copyright (c) 2025 grrimgrriefer & DZnnah. See LICENSE in root for details.
