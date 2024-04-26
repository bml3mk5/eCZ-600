/** @file cocoa_savedatarec.mm

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2017.12.29 -

 @brief [ save datarec panel ]
 */

#import "cocoa_savedatarec.h"
#import "../../clocale.h"
#import "../../config.h"
#import "../../msgs.h"

@implementation CocoaSaveDatarec
- (id)init
{
	[super init];

	// title
	[self setTitle:[NSString stringWithUTF8String:CMSG(Record_Data_Recorder_Tape)]];
	// filtering file types
	NSArray *fileTypes = [NSArray arrayWithObjects:@"l3",nil];
	[self setAllowedFileTypes:fileTypes];
	[self setAllowsOtherFileTypes:YES];
	[self setExtensionHidden:NO];
	// set current folder
	[self setDirectoryURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:pConfig->GetInitialDataRecPath()]]];

	//
	NSRect re = NSMakeRect(0, 0, 380, 26);
	NSView *view = [[NSView alloc] initWithFrame:re];

	re = NSMakeRect(0, -4, 80, 26);
	CocoaLabel *lbl = [CocoaLabel createWithoutFit:re titleid:CMsg::File_Type_COLON align:NSTextAlignmentLeft];
	[view addSubview:lbl];

	const char *lst[] = {
		"l3", "l3b", "l3c", "wav", "t9x",
		NULL
	};
	re = NSMakeRect(80, 0, 300, 26);
	CocoaRadioGroup *rad = [CocoaRadioGroup create:re rows:1 cols:5 titles:lst action:@selector(changeFileType:) selidx:0];
	[view addSubview:rad];

	[self setAccessoryView:view];

	return self;
}

- (void)changeFileType:(CocoaRadioGroup *)sender
{
	int num = (int)[sender selectedColumn];
	NSArray *fileTypes;
	switch(num) {
		case 1:
			fileTypes = [NSArray arrayWithObjects:@"l3b",nil];
			break;
		case 2:
			fileTypes = [NSArray arrayWithObjects:@"l3c",nil];
			break;
		case 3:
			fileTypes = [NSArray arrayWithObjects:@"wav",nil];
			break;
		case 4:
			fileTypes = [NSArray arrayWithObjects:@"t9x",nil];
			break;
		default:
			fileTypes = [NSArray arrayWithObjects:@"l3",nil];
			break;
	}
	[self setAllowedFileTypes:fileTypes];
}

@end
