//
//  ViewController.m
//  HTML2IOS
//
//  Created by Viren Vijay Mane on 7/11/25.
//

#import "ViewController.h"
#import <WebKit/WebKit.h>

@interface ViewController () <WKUIDelegate>

@property (nonatomic, strong) WKWebView *webView;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // Setup web view
    WKWebViewConfiguration *config = [[WKWebViewConfiguration alloc] init];
    self.webView = [[WKWebView alloc] initWithFrame:self.view.bounds configuration:config];
    self.webView.UIDelegate = self;
    self.webView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self.view addSubview:self.webView];

    // Load local HTML file
    NSString *filePath = [[NSBundle mainBundle] pathForResource:@"index" ofType:@"html"];
    if (filePath) {
        NSURL *fileURL = [NSURL fileURLWithPath:filePath];
        NSURL *dirURL = [fileURL URLByDeletingLastPathComponent];
        [self.webView loadFileURL:fileURL allowingReadAccessToURL:dirURL];
    } else {
        NSLog(@"[ERROR] Could not find HTML2IOSA/website/index.html in bundle.");
    }
}

#pragma mark - WKUIDelegate (for JS alert/confirm/prompt)

- (void)webView:(WKWebView *)webView
runJavaScriptAlertPanelWithMessage:(NSString *)message
initiatedByFrame:(WKFrameInfo *)frame
completionHandler:(void (^)(void))completionHandler {

    UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"Alert"
                                                                   message:message
                                                            preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"OK"
                                              style:UIAlertActionStyleDefault
                                            handler:^(UIAlertAction * _Nonnull action) {
        completionHandler();
    }]];
    [self presentViewController:alert animated:YES completion:nil];
}

- (void)webView:(WKWebView *)webView
runJavaScriptConfirmPanelWithMessage:(NSString *)message
initiatedByFrame:(WKFrameInfo *)frame
completionHandler:(void (^)(BOOL result))completionHandler {

    UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"Confirm"
                                                                   message:message
                                                            preferredStyle:UIAlertControllerStyleAlert];

    [alert addAction:[UIAlertAction actionWithTitle:@"Cancel"
                                              style:UIAlertActionStyleCancel
                                            handler:^(UIAlertAction * _Nonnull action) {
        completionHandler(NO);
    }]];

    [alert addAction:[UIAlertAction actionWithTitle:@"OK"
                                              style:UIAlertActionStyleDefault
                                            handler:^(UIAlertAction * _Nonnull action) {
        completionHandler(YES);
    }]];

    [self presentViewController:alert animated:YES completion:nil];
}

- (void)webView:(WKWebView *)webView
runJavaScriptTextInputPanelWithPrompt:(NSString *)prompt
defaultText:(NSString *)defaultText
initiatedByFrame:(WKFrameInfo *)frame
completionHandler:(void (^)(NSString * _Nullable result))completionHandler {

    UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"Prompt"
                                                                   message:prompt
                                                            preferredStyle:UIAlertControllerStyleAlert];

    [alert addTextFieldWithConfigurationHandler:^(UITextField * _Nonnull textField) {
        textField.text = defaultText;
    }];

    [alert addAction:[UIAlertAction actionWithTitle:@"Cancel"
                                              style:UIAlertActionStyleCancel
                                            handler:^(UIAlertAction * _Nonnull action) {
        completionHandler(nil);
    }]];

    [alert addAction:[UIAlertAction actionWithTitle:@"OK"
                                              style:UIAlertActionStyleDefault
                                            handler:^(UIAlertAction * _Nonnull action) {
        NSString *input = alert.textFields.firstObject.text;
        completionHandler(input);
    }]];

    [self presentViewController:alert animated:YES completion:nil];
}

@end
