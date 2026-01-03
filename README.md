# Logic Pro Discord Rich Presence

A lightweight macOS menu-bar app that shows your **Logic Pro** activity as a **Discord Rich Presence**.

When Logic Pro is open:
- Shows **“Browsing projects”** if no project is open
- Shows **“Working on <project name>”** when a project is loaded

When Logic Pro is closed:
- Discord status is cleared

<img width="276" height="103" alt="Screenshot 2025-12-31 at 1 17 30 PM" src="https://github.com/user-attachments/assets/da991686-8f32-462d-9d70-712d7c57163c" />
<img width="170" height="136" alt="Screenshot 2025-12-31 at 1 24 10 PM" src="https://github.com/user-attachments/assets/c0aa2a48-92df-4959-af68-704f5b82b071" />

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
- re-build rpc to use discord-rpc express (remove need for discord application & runs locally)
- two instances of error handling (currently caught exception)
- menu bar icon (currently text)
- filesize optimization
- more coming soon...

Please message me on Discord @beastrg if there are features you would like to see or if you encounter bugs (git issues work as well).
