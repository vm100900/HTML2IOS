# HTML2IOSA

**Convert your local HTML projects into fully functional native iOS apps — fast, easy, and offline-ready.**

---

## What is HTML2IOSA?

HTML2IOSA is a developer-focused tool that **generates a complete Xcode iOS app project from your local HTML files**, bundling everything so your app runs offline with zero hassle.

If you’ve got an `index.html` and assets that open locally on your laptop, HTML2IOSA wraps them in a native iOS shell using WKWebView — no complex native UI coding required.

---

## Key Features

- **Full Xcode project generation** ready to build and run  
- Bundles all your HTML, CSS, JS, and assets for offline use  
- Runs your web UI inside a performant native WKWebView  
- Designed for HTML developers, no native iOS experience required  
- iOS-only, runs on macOS machines with Xcode installed  
- Future-ready: v0.2 will add native feature hooks and Info.plist customization  

---

## Current Limitations

- No CLI support yet for customizing bundle ID or app name (coming soon in v0.2)  
- Native feature support (camera, notifications, etc.) will arrive with v0.2 via automatic Info.plist patching  
- Requires your project to have an `index.html` that loads locally without a web server  

---

## Installation

### Download or build the binary

[Add your instructions here for downloading the pre-built binary or building from source.]

### Add `html2iosa` to your PATH

To run the tool easily from anywhere in your terminal, add its folder to your PATH:

1. Locate where the `html2iosa` binary is saved, for example: `/usr/local/bin/html2iosa`

2. Add the directory to your PATH environment variable.

For **bash** or **zsh**, add this line to your `~/.bash_profile` or `~/.zshrc`:

export PATH="/usr/local/bin:$PATH"
Reload your shell configuration:

bash
Copy
Edit
source ~/.bash_profile
# or
source ~/.zshrc
Test by running:

bash
Copy
Edit
html2iosa --help
If you see usage information, you're good to go.

How to Use
Prerequisites
macOS with Xcode installed

Your local HTML project folder with index.html

Generate your iOS app
Run the tool pointing at your HTML folder:

bash
Copy
Edit
html2iosa --input ./my-html-project --output ./MyiOSApp
This creates a complete Xcode project at ./MyiOSApp with all your assets bundled.

Build & Run
Open MyiOSApp/MyiOSApp.xcodeproj in Xcode

Build and run on the iOS simulator or a real device

What’s Coming in v0.2?
Command-line options to customize app name and bundle ID

Direct editing of Info.plist for metadata configuration

Support for native iOS features like camera, microphone, and notifications, automatically enabled if your website uses them

Better integration with native APIs via JavaScript bridge

Improved offline caching and app icon/splash screen customization

Why Use HTML2IOSA?
If you’re a web developer who:

Wants to quickly ship your HTML app as an iOS app without learning Swift

Needs offline support baked in automatically

Wants a full Xcode project you can extend later with native code

Appreciates automation and saving weeks of development time

This tool is your launchpad.

Troubleshooting & Tips
Ensure your index.html and all referenced assets open correctly from the local filesystem

The app loads your HTML inside WKWebView — complex SPAs may have performance limits

Watch out for absolute URLs that require network access if offline use is essential

Use Xcode logs for debugging native or WebView issues

License
MIT — use it however you want, improve it, break it, share it.

Contributing & Feedback
Open source, open doors. Submit issues or PRs on the repo to help make this the best HTML-to-iOS conversion tool out there.

Ready to turn your web projects into native iOS apps? HTML2IOSA gets you there without the native learning curve.

vbnet
Copy
Edit

If you want me to draft the download/build instructions or CLI u
