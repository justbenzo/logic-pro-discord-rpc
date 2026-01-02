#import <Cocoa/Cocoa.h>
#include "engine.h"

static NSString * const kLaunchAgentLabel = @"com.benzo.logicrpc";
static NSString * LaunchAgentPlistPath() {
    return [NSHomeDirectory() stringByAppendingPathComponent:
            @"Library/LaunchAgents/com.benzo.logicrpc.plist"];
}

static NSString * ExecutablePath() {
    return [[NSBundle mainBundle] executablePath];
}

static NSString * PlistContents(NSString *execPath) {
    return [NSString stringWithFormat:
        @"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
         "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
         "<plist version=\"1.0\">\n"
         "<dict>\n"
         "  <key>Label</key>\n"
         "  <string>%@</string>\n"
         "  <key>ProgramArguments</key>\n"
         "  <array>\n"
         "    <string>%@</string>\n"
         "  </array>\n"
         "  <key>RunAtLoad</key>\n"
         "  <true/>\n"
         "  <key>KeepAlive</key>\n"
         "  <false/>\n"
         "</dict>\n"
         "</plist>\n",
         kLaunchAgentLabel, execPath];
}

static int RunLaunchctl(NSArray<NSString*> *args) {
    NSTask *task = [[NSTask alloc] init];
    task.launchPath = @"/bin/launchctl";
    task.arguments = args;

    @try {
        [task launch];
        [task waitUntilExit];
        return task.terminationStatus;
    } @catch (NSException *ex) {
        return 1;
    }
}

static BOOL IsStartAtLoginEnabled() {
    return [[NSFileManager defaultManager] fileExistsAtPath:LaunchAgentPlistPath()];
}

static BOOL EnableStartAtLogin() {
    NSString *dir = [LaunchAgentPlistPath() stringByDeletingLastPathComponent];
    [[NSFileManager defaultManager] createDirectoryAtPath:dir
                              withIntermediateDirectories:YES
                                               attributes:nil
                                                    error:nil];

    NSString *plist = PlistContents(ExecutablePath());
    NSError *err = nil;
    BOOL ok = [plist writeToFile:LaunchAgentPlistPath()
                      atomically:YES
                        encoding:NSUTF8StringEncoding
                           error:&err];
    return ok;
}


static void DisableStartAtLogin() {
    NSString *uidStr = [NSString stringWithFormat:@"%d", getuid()];
    NSString *domain = [NSString stringWithFormat:@"gui/%@", uidStr];

    RunLaunchctl(@[@"bootout", domain, LaunchAgentPlistPath()]);
    [[NSFileManager defaultManager] removeItemAtPath:LaunchAgentPlistPath() error:nil];
}

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (strong) NSStatusItem *statusItem;
@property (strong) NSMenuItem *startAtLoginItem;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];

    self.statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
    if (self.statusItem.button) {
        self.statusItem.button.title = @"lgrp";
        self.statusItem.button.toolTip = @"Logic RPC";
    }

    NSMenu *menu = [[NSMenu alloc] init];

    self.startAtLoginItem = [[NSMenuItem alloc] initWithTitle:@"Start at Login"
                                                       action:@selector(toggleStartAtLogin)
                                                keyEquivalent:@""];
    self.startAtLoginItem.target = self;
    self.startAtLoginItem.state = IsStartAtLoginEnabled() ? NSControlStateValueOn : NSControlStateValueOff;
    [menu addItem:self.startAtLoginItem];

    [menu addItem:[NSMenuItem separatorItem]];

    NSMenuItem *authItem = [[NSMenuItem alloc] initWithTitle:@"Authenticate"
                                                      action:@selector(authenticate)
                                               keyEquivalent:@""];
    authItem.target = self;
    [menu addItem:authItem];

    [menu addItem:[NSMenuItem separatorItem]];

    NSMenuItem *quitItem = [[NSMenuItem alloc] initWithTitle:@"Quit"
                                                      action:@selector(quit)
                                               keyEquivalent:@"q"];
    quitItem.target = self;
    [menu addItem:quitItem];

    self.statusItem.menu = menu;

    engine::start_idle();
}

- (void)toggleStartAtLogin {
    if (self.startAtLoginItem.state == NSControlStateValueOn) {
        DisableStartAtLogin();
        self.startAtLoginItem.state = NSControlStateValueOff;
    } else {
        BOOL ok = EnableStartAtLogin();
        self.startAtLoginItem.state = ok ? NSControlStateValueOn : NSControlStateValueOff;

        if (!ok) {
            NSBeep();
        }
    }
}

- (void)authenticate {
    engine::reset_auth_and_authenticate();
}


- (void)quit {
    engine::stop();
    [NSApp terminate:nil];
}

@end

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        AppDelegate *delegate = [AppDelegate new];
        app.delegate = delegate;
        NSString *bid = [[NSBundle mainBundle] bundleIdentifier];
        if (bid) {
            NSArray<NSRunningApplication *> *apps =
                [NSRunningApplication runningApplicationsWithBundleIdentifier:bid];

            if (apps.count > 1) {
                return 0;
            }
        }
        [app run];
    }
    return 0;
}
