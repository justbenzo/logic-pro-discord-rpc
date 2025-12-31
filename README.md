# Logic Pro Discord Rich Presence

A lightweight macOS menu-bar app that shows your **Logic Pro** activity as a **Discord Rich Presence**.

When Logic Pro is open:
- Shows **“Browsing projects”** if no project is open
- Shows **“Working on <project name>”** when a project is loaded

When Logic Pro is closed:
- Discord status is cleared

---

## Features

- macOS menu-bar app
- Silent re-authentication (no login every launch)
- Optional **Start at Login** checkbox

---

## Download & Install (macOS)

1. Download the latest release from **GitHub Releases**
2. Unzip `logicrpc.app`
3. Move it to `/Applications`
4. Right-click the app → **Open** (first launch only, Gatekeeper)
5. The app will appear in your menu bar

---

## First-Time Setup

1. Click the menu-bar icon
2. Click **Authenticate…**
3. Log in to Discord when prompted

You only need to do this once.

---

## Start at Login

Use the menu-bar checkbox **“Start at Login”** to enable or disable launching automatically when you log in.

---

## Build from Source (Advanced)

Requirements:
- macOS
- CMake
- Xcode Command Line Tools

### Discord SDK (required)
The Discord **Social SDK** binary is not included (licensing).

Download the SDK and place it at:
lib/discord_social_sdk/lib/release/libdiscord_partner_sdk.dylib

Then build:
```bash
cmake -S . -B build
cmake --build build
```

To-Do
- two instances of error handling (currently caught exception)
- menu bar icon (currently text)
- filesize optimization
- more coming soon...

Please message me on Discord @beastrg if there are features you would like to see or if you encounter bugs (git issues work as well).
