/** @file cocoa_basepanel.mm

 Skelton for retropc emulator
 SDL edition + Cocoa GUI

 @author Sasaji
 @date   2015.04.30 -

 @brief [ panel ]
 */

#import "cocoa_basepanel.h"
#import "../../utility.h"

//#define _DEBUG_CBOX
#ifdef _DEBUG_CBOX
#include "../../emu.h"
extern EMU *emu;
#endif


/**
	Text size on control
*/
@implementation CocoaTextSize
+ (NSSize)size:(NSControl *)ctrl constTo:(NSSize)constraint
{
	NSFont *font = [ctrl font];

	NSTextStorage *textStorage = [[NSTextStorage alloc] initWithString:@""];
	NSTextContainer *textContainer = [[NSTextContainer alloc] initWithContainerSize:constraint];

	NSLayoutManager *layoutManager = [[NSLayoutManager alloc] init];
	[layoutManager addTextContainer:textContainer];
	[textStorage addLayoutManager:layoutManager];
	[textStorage addAttribute:NSFontAttributeName value:font
						range:NSMakeRange(0, [textStorage length])];
	[textContainer setLineFragmentPadding:0.0];

	(void) [layoutManager glyphRangeForTextContainer:textContainer];
	return [layoutManager
			usedRectForTextContainer:textContainer].size;
}
@end


/**
	Static label
*/
@implementation CocoaLabel
+ (CocoaLabel *)create:(NSRect)re title:(const char *)title
{
	return [CocoaLabel createWithFit:&re title:title align:NSTextAlignmentLeft];
}
+ (CocoaLabel *)create:(NSRect)re titleid:(CMsg::Id)titleid
{
	return [CocoaLabel createWithFit:&re titleid:titleid align:NSTextAlignmentLeft];
}
+ (CocoaLabel *)create:(NSRect)re title:(const char *)title align:(NSTextAlignment)align
{
	return [CocoaLabel createWithFit:&re title:title align:align];
}
+ (CocoaLabel *)create:(NSRect)re titleid:(CMsg::Id)titleid align:(NSTextAlignment)align
{
	return [CocoaLabel createWithFit:&re titleid:titleid align:align];
}
+ (CocoaLabel *)createT:(const char *)title
{
	return [CocoaLabel createT:title align:NSTextAlignmentLeft];
}
+ (CocoaLabel *)createI:(CMsg::Id)titleid
{
	return [CocoaLabel createT:CMSGV(titleid) align:NSTextAlignmentLeft];
}
+ (CocoaLabel *)createT:(const char *)title align:(NSTextAlignment)align
{
	CocoaLabel *me = [CocoaLabel alloc];

	[me init];
	[me setStringValue:[NSString stringWithUTF8String:title]];
	[me setEditable:FALSE];
	[me setBezeled:FALSE];
	[me setDrawsBackground:FALSE];
	[me setSelectable:FALSE];
	[me setAlignment:align];
	[me sizeToFit];

	return me;
}
+ (CocoaLabel *)createI:(CMsg::Id)titleid align:(NSTextAlignment)align
{
	return [CocoaLabel createT:CMSGV(titleid) align:align];
}
+ (CocoaLabel *)createT:(CocoaLayout *)layout title:(const char *)title
{
	CocoaLabel *me = [CocoaLabel createT:title];
	[layout addControl:me];
	return me;
}
+ (CocoaLabel *)createI:(CocoaLayout *)layout title:(CMsg::Id)titleid
{
	CocoaLabel *me = [CocoaLabel createI:titleid];
	[layout addControl:me];
	return me;
}
+ (CocoaLabel *)createT:(CocoaLayout *)layout title:(const char *)title width:(int)width
{
	CocoaLabel *me = [CocoaLabel createT:title];
	[layout addControl:me width:width];
	return me;
}
+ (CocoaLabel *)createI:(CocoaLayout *)layout title:(CMsg::Id)titleid width:(int)width
{
	CocoaLabel *me = [CocoaLabel createI:titleid];
	[layout addControl:me width:width];
	return me;
}
+ (CocoaLabel *)createT:(CocoaLayout *)layout title:(const char *)title width:(int)width height:(int)height
{
	CocoaLabel *me = [CocoaLabel createT:title];
	[layout addControl:me width:width height:height];
	return me;
}
+ (CocoaLabel *)createI:(CocoaLayout *)layout title:(CMsg::Id)titleid width:(int)width height:(int)height
{
	CocoaLabel *me = [CocoaLabel createI:titleid];
	[layout addControl:me width:width height:height];
	return me;
}
+ (CocoaLabel *)createT:(CocoaLayout *)layout title:(const char *)title align:(NSTextAlignment)align
{
	CocoaLabel *me = [CocoaLabel createT:title align:align];
	[layout addControl:me];
	return me;
}
+ (CocoaLabel *)createI:(CocoaLayout *)layout title:(CMsg::Id)titleid align:(NSTextAlignment)align
{
	CocoaLabel *me = [CocoaLabel createI:titleid align:align];
	[layout addControl:me];
	return me;
}
+ (CocoaLabel *)createT:(CocoaLayout *)layout title:(const char *)title align:(NSTextAlignment)align width:(int)width height:(int)height
{
	CocoaLabel *me = [CocoaLabel createT:title align:align];
	[layout addControl:me width:width height:height];
	return me;
}
+ (CocoaLabel *)createI:(CocoaLayout *)layout title:(CMsg::Id)titleid align:(NSTextAlignment)align width:(int)width height:(int)height
{
	CocoaLabel *me = [CocoaLabel createI:titleid align:align];
	[layout addControl:me width:width height:height];
	return me;
}
+ (CocoaLabel *)createWithFitW:(NSRect *)re title:(const char *)title
{
	return [CocoaLabel createWithFitW:re title:title align:NSTextAlignmentLeft];
}
+ (CocoaLabel *)createWithFitW:(NSRect *)re titleid:(CMsg::Id)titleid
{
	return [CocoaLabel createWithFitW:re titleid:titleid align:NSTextAlignmentLeft];
}
+ (CocoaLabel *)createWithFitW:(NSRect *)re title:(const char *)title align:(NSTextAlignment)align
{
	NSRect rew = *re;
	CocoaLabel *obj = [CocoaLabel createWithFit:&rew title:title align:align];
	re->size.width = rew.size.width;
	[obj setFrame:*re];
	return obj;
}
+ (CocoaLabel *)createWithFitW:(NSRect *)re titleid:(CMsg::Id)titleid align:(NSTextAlignment)align
{
	return [CocoaLabel createWithFitW:re title:gMessages.Get(titleid) align:align];
}
+ (CocoaLabel *)createWithFitH:(NSRect *)re title:(const char *)title
{
	return [CocoaLabel createWithFitH:re title:title align:NSTextAlignmentLeft];
}
+ (CocoaLabel *)createWithFitH:(NSRect *)re title:(const char *)title align:(NSTextAlignment)align
{
	NSRect reh = *re;
	CocoaLabel *obj = [CocoaLabel createWithFit:&reh title:title align:align];
	re->size.height = reh.size.height;
	[obj setFrame:*re];
	return obj;
}
+ (CocoaLabel *)createWithFit:(NSRect *)re title:(const char *)title
{
	return [CocoaLabel createWithFit:re title:title align:NSTextAlignmentLeft];
}
+ (CocoaLabel *)createWithFit:(NSRect *)re titleid:(CMsg::Id)titleid
{
	return [CocoaLabel createWithFit:re titleid:titleid align:NSTextAlignmentLeft];
}
+ (CocoaLabel *)createWithFit:(NSRect *)re title:(const char *)title align:(NSTextAlignment)align
{
	CocoaLabel *me = [CocoaLabel alloc];

	[me initWithFrame:*re];
	[me setStringValue:[NSString stringWithUTF8String:title]];
	[me setEditable:FALSE];
	[me setBezeled:FALSE];
	[me setDrawsBackground:FALSE];
	[me setSelectable:FALSE];
	[me setAlignment:align];
	[me sizeToFit];
	*re = [me frame];

	return me;
}
+ (CocoaLabel *)createWithFit:(NSRect *)re titleid:(CMsg::Id)titleid align:(NSTextAlignment)align
{
	return [CocoaLabel createWithFit:re title:gMessages.Get(titleid) align:align];
}
+ (CocoaLabel *)createWithoutFit:(NSRect)re title:(const char *)title align:(NSTextAlignment)align
{
	CocoaLabel *me = [CocoaLabel alloc];

	[me initWithFrame:re];
	[me setStringValue:[NSString stringWithUTF8String:title]];
	[me setEditable:FALSE];
	[me setBezeled:FALSE];
	[me setDrawsBackground:FALSE];
	[me setSelectable:FALSE];
	[me setAlignment:align];

	return me;
}
+ (CocoaLabel *)createWithoutFit:(NSRect)re titleid:(CMsg::Id)titleid align:(NSTextAlignment)align
{
	return [CocoaLabel createWithoutFit:re title:gMessages.Get(titleid) align:align];
}
@end


/**
	Text field
*/
@implementation CocoaTextField
#ifdef COCOA_USE_OLDSTYLE_LAYOUT
+ (CocoaTextField *)create:(NSRect)re num:(int)num action:(SEL)action
{
	return [CocoaTextField create:re num:num action:action align:NSLeftTextAlignment];
}
+ (CocoaTextField *)create:(NSRect)re num:(int)num action:(SEL)action align:(NSTextAlignment)align
{
	CocoaTextField *me = [CocoaTextField alloc];

	[me initWithFrame:re];
	[me setStringValue:[[NSNumber numberWithInt:num] stringValue]];
	[me setEditable:YES];
	[me setBezeled:YES];
	[me setDrawsBackground:YES];
	[me setSelectable:YES];
	[me setAction:action];
	[me setAlignment:align];
	/*	[[me cell] setUsesSingleLineMode:YES]; */

	return me;
}
+ (CocoaTextField *)create:(NSRect)re text:(const char *)text action:(SEL)action
{
	return [CocoaTextField create:re text:text action:action align:NSLeftTextAlignment];
}
+ (CocoaTextField *)create:(NSRect)re text:(const char *)text action:(SEL)action align:(NSTextAlignment)align
{
	CocoaTextField *me = [CocoaTextField alloc];

	[me initWithFrame:re];
	[me setStringValue:[NSString stringWithUTF8String:text]];
	[me setEditable:YES];
	[me setBezeled:YES];
	[me setDrawsBackground:YES];
	[me setSelectable:YES];
	[me setAction:action];
	[me setAlignment:align];
/*	[[me cell] setUsesSingleLineMode:YES]; */

	return me;
}
+ (CocoaTextField *)create:(NSRect)re textid:(CMsg::Id)textid action:(SEL)action
{
	return [CocoaTextField create:re text:CMSGV(textid) action:action];
}
+ (CocoaTextField *)create:(NSRect)re textid:(CMsg::Id)textid action:(SEL)action align:(NSTextAlignment)align
{
	return [CocoaTextField create:re text:CMSGV(textid) action:action align:align];
}
#endif

+ (CocoaTextField *)createN:(int)num action:(SEL)action
{
	return [CocoaTextField createN:num action:action align:NSTextAlignmentLeft];
}
+ (CocoaTextField *)createN:(int)num action:(SEL)action align:(NSTextAlignment)align
{
	CocoaTextField *me = [CocoaTextField alloc];

	[me init];
	[me setStringValue:[[NSNumber numberWithInt:num] stringValue]];
	[me setEditable:YES];
	[me setBezeled:YES];
	[me setDrawsBackground:YES];
	[me setSelectable:YES];
	[me setAction:action];
	[me setAlignment:align];
	/*	[[me cell] setUsesSingleLineMode:YES]; */

	return me;
}
+ (CocoaTextField *)createT:(const char *)text action:(SEL)action
{
	return [CocoaTextField createT:text action:action align:NSTextAlignmentLeft];
}
+ (CocoaTextField *)createT:(const char *)text action:(SEL)action align:(NSTextAlignment)align
{
	CocoaTextField *me = [CocoaTextField alloc];

	[me init];
	[me setStringValue:[NSString stringWithUTF8String:text]];
	[me setEditable:YES];
	[me setBezeled:YES];
	[me setDrawsBackground:YES];
	[me setSelectable:YES];
	[me setAction:action];
	[me setAlignment:align];
	/*	[[me cell] setUsesSingleLineMode:YES]; */

	return me;
}
+ (CocoaTextField *)createI:(CMsg::Id)textid action:(SEL)action
{
	return [CocoaTextField createT:CMSGV(textid) action:action align:NSTextAlignmentLeft];
}
+ (CocoaTextField *)createI:(CMsg::Id)textid action:(SEL)action align:(NSTextAlignment)align
{
	return [CocoaTextField createT:CMSGV(textid) action:action align:align];
}
+ (CocoaTextField *)createN:(CocoaLayout *)layout num:(int)num action:(SEL)action width:(int)width
{
	CocoaTextField *me = [CocoaTextField createN:num action:action];
	[layout addControl:me width:width];
	return me;
}
+ (CocoaTextField *)createN:(CocoaLayout *)layout num:(int)num action:(SEL)action align:(NSTextAlignment)align width:(int)width
{
	CocoaTextField *me = [CocoaTextField createN:num action:action align:align];
	[layout addControl:me width:width];
	return me;
}
+ (CocoaTextField *)createT:(CocoaLayout *)layout text:(const char *)text action:(SEL)action width:(int)width
{
	CocoaTextField *me = [CocoaTextField createT:text action:action];
	[layout addControl:me width:width];
	return me;
}
+ (CocoaTextField *)createT:(CocoaLayout *)layout text:(const char *)text action:(SEL)action align:(NSTextAlignment)align width:(int)width
{
	CocoaTextField *me = [CocoaTextField createT:text action:action align:align];
	[layout addControl:me width:width];
	return me;
}
+ (CocoaTextField *)createI:(CocoaLayout *)layout text:(CMsg::Id)textid action:(SEL)action width:(int)width
{
	CocoaTextField *me = [CocoaTextField createI:textid action:action];
	[layout addControl:me width:width];
	return me;
}
+ (CocoaTextField *)createI:(CocoaLayout *)layout text:(CMsg::Id)textid action:(SEL)action align:(NSTextAlignment)align width:(int)width
{
	CocoaTextField *me = [CocoaTextField createI:textid action:action align:align];
	[layout addControl:me width:width];
	return me;
}
+ (CocoaTextField *)create:(CocoaLayout *)layout action:(SEL)action width:(int)width height:(int)height
{
	CocoaTextField *me = [CocoaTextField alloc];

	[me init];
//	[me setStringValue:[NSString stringWithUTF8String:text]];
	[me setEditable:NO];
	[me setBezeled:YES];
	[me setDrawsBackground:YES];
	[me setSelectable:YES];
	[me setAction:action];
//	[me setAlignment:align];
	/*	[[me cell] setUsesSingleLineMode:YES]; */
	[layout addControl:me width:width height:height];

	return me;
}
@end


/**
	Text view
*/
@implementation CocoaTextView : NSTextView
@synthesize parent;
+ (CocoaTextView *)create:(CocoaLayout *)layout edit:(bool)edit hasvs:(bool)hasvs hashs:(bool)hashs width:(int)width height:(int)height
{
	CocoaScrollView *sview = [CocoaScrollView create:layout hasvs:hasvs hashs:hashs width:width height:height];

	NSRect re = NSMakeRect(0,0,width,height);
	CocoaTextView *me = [[CocoaTextView alloc] initWithFrame:re];
	[me setEditable:edit];
	[me setDrawsBackground:YES];
	[me setSelectable:YES];

	[sview setDocumentView:me];
	[me setParent:sview];
	return me;
}
@end


/**
	for button control
*/
@implementation CocoaObjectStructure
@synthesize obj1;
@synthesize obj2;
+ (CocoaObjectStructure *)create:(id)newobj1 :(id)newobj2
{
	CocoaObjectStructure *me = [CocoaObjectStructure alloc];
	[me init];
	me.obj1=newobj1;
	me.obj2=newobj2;

	return me;
}
@end


/**
	Button control
*/
@implementation CocoaButton
@synthesize relatedObject;
+ (CocoaButton *)create:(NSRect)re title:(const char *)title action:(SEL)action
{
	CocoaButton *me = [CocoaButton alloc];

	[me initWithFrame:re];
	[me setTitle:[NSString stringWithUTF8String:title]];
	[me setBezelStyle:NSRoundedBezelStyle];
	[me setAction:action];
	[me setRelatedObject:nil];

	return me;
}
+ (CocoaButton *)create:(NSRect)re titleid:(CMsg::Id)titleid action:(SEL)action
{
	return [CocoaButton create:re title:gMessages.Get(titleid) action:action];
}
+ (CocoaButton *)createT:(const char *)title action:(SEL)action
{
	CocoaButton *me = [CocoaButton alloc];

	[me init];
	[me setTitle:[NSString stringWithUTF8String:title]];
	[me setBezelStyle:NSRoundedBezelStyle];
	[me setAction:action];
	[me setRelatedObject:nil];

	return me;
}
+ (CocoaButton *)createI:(CMsg::Id)titleid action:(SEL)action
{
	return [CocoaButton createT:CMSGV(titleid) action:action];
}
+ (CocoaButton *)createT:(CocoaLayout *)layout title:(const char *)title action:(SEL)action
{
	CocoaButton *me = [CocoaButton createT:title action:action];
	[layout addControl:me];
	return me;
}
+ (CocoaButton *)createI:(CocoaLayout *)layout title:(CMsg::Id)titleid action:(SEL)action
{
	CocoaButton *me = [CocoaButton createI:titleid action:action];
	[layout addControl:me];
	return me;
}
+ (CocoaButton *)createT:(CocoaLayout *)layout title:(const char *)title action:(SEL)action width:(int)width
{
	CocoaButton *me = [CocoaButton createT:title action:action];
	[layout addControl:me width:width];
	return me;
}
+ (CocoaButton *)createI:(CocoaLayout *)layout title:(CMsg::Id)titleid action:(SEL)action width:(int)width
{
	CocoaButton *me = [CocoaButton createI:titleid action:action];
	[layout addControl:me width:width];
	return me;
}
@end


/**
	Popup button
*/
@implementation CocoaPopUpButton
+ (CocoaPopUpButton *)create:(NSRect)re items:(const char **)items action:(SEL)action selidx:(int)selidx
{
	CocoaPopUpButton *me = [CocoaPopUpButton alloc];
	[me initWithFrame:re pullsDown:NO];
	[me addItemsT:items selidx:selidx];
	[me setAction:action];
	return me;
}
+ (CocoaPopUpButton *)create:(NSRect)re itemids:(const CMsg::Id *)itemids action:(SEL)action selidx:(int)selidx
{
	CocoaPopUpButton *me = [CocoaPopUpButton alloc];
	[me initWithFrame:re pullsDown:NO];
	[me addItemsI:itemids selidx:selidx];
	[me setAction:action];
	return me;
}
+ (CocoaPopUpButton *)createT:(const char **)items action:(SEL)action selidx:(int)selidx
{
	CocoaPopUpButton *me = [CocoaPopUpButton alloc];
	[me init];
	[me setPullsDown:NO];
	[me addItemsT:items selidx:selidx];
	[me setAction:action];
	return me;
}
+ (CocoaPopUpButton *)createI:(const CMsg::Id *)itemids action:(SEL)action selidx:(int)selidx
{
	CocoaPopUpButton *me;
	me = [CocoaPopUpButton createI:itemids action:action selidx:selidx appendnum:-1 appendstr:CMsg::End];
	return me;
}
+ (CocoaPopUpButton *)createI:(const CMsg::Id *)itemids action:(SEL)action selidx:(int)selidx appendnum:(int)appendnum appendstr:(CMsg::Id)appendstr
{
	CocoaPopUpButton *me = [CocoaPopUpButton alloc];
	[me init];
	[me setPullsDown:NO];
	[me addItemsI:itemids selidx:selidx appendnum:appendnum appendstr:appendstr];
	[me setAction:action];
	return me;
}
+ (CocoaPopUpButton *)createL:(const CPtrList<CTchar> *)items action:(SEL)action selidx:(int)selidx
{
	CocoaPopUpButton *me = [CocoaPopUpButton alloc];
	[me init];
	[me setPullsDown:NO];
	[me addItemsL:items selidx:selidx];
	[me setAction:action];
	return me;
}
+ (CocoaPopUpButton *)createT:(CocoaLayout *)layout items:(const char **)items action:(SEL)action selidx:(int)selidx
{
	CocoaPopUpButton *me = [CocoaPopUpButton createT:items action:action selidx:selidx];
	[layout addControl:me];
	return me;
}
+ (CocoaPopUpButton *)createT:(CocoaLayout *)layout items:(const char **)items action:(SEL)action selidx:(int)selidx width:(int)width
{
	CocoaPopUpButton *me = [CocoaPopUpButton createT:items action:action selidx:selidx];
	[layout addControl:me width:width];
	return me;
}
+ (CocoaPopUpButton *)createI:(CocoaLayout *)layout items:(const CMsg::Id *)itemids action:(SEL)action selidx:(int)selidx
{
	CocoaPopUpButton *me = [CocoaPopUpButton createI:itemids action:action selidx:selidx];
	[layout addControl:me];
	return me;
}
+ (CocoaPopUpButton *)createI:(CocoaLayout *)layout items:(const CMsg::Id *)itemids action:(SEL)action selidx:(int)selidx width:(int)width
{
	CocoaPopUpButton *me = [CocoaPopUpButton createI:itemids action:action selidx:selidx];
	[layout addControl:me width:width];
	return me;
}
+ (CocoaPopUpButton *)createI:(CocoaLayout *)layout items:(const CMsg::Id *)itemids action:(SEL)action selidx:(int)selidx appendnum:(int)appendnum appendstr:(CMsg::Id)appendstr
{
	CocoaPopUpButton *me = [CocoaPopUpButton createI:itemids action:action selidx:selidx appendnum:appendnum appendstr:appendstr];
	[layout addControl:me];
	return me;
}
+ (CocoaPopUpButton *)createL:(CocoaLayout *)layout items:(const CPtrList<CTchar> *)items action:(SEL)action selidx:(int)selidx
{
	CocoaPopUpButton *me = [CocoaPopUpButton createL:items action:action selidx:selidx];
	[layout addControl:me];
	return me;
}
- (void)addItemsT:(const char **)items selidx:(int)selidx
{
	int i;
	for(i=0; items[i] != NULL; i++) {
		[self addItemWithTitle:[NSString stringWithUTF8String:items[i]]];
	}
	if (selidx >= 0 && selidx < i) {
		[self selectItemAtIndex:selidx];
	}
}
- (void)addItemsI:(const CMsg::Id *)itemids selidx:(int)selidx
{
	[self addItemsI:itemids selidx:selidx appendnum:-1 appendstr:CMsg::End];
}
- (void)addItemsI:(const CMsg::Id *)itemids selidx:(int)selidx appendnum:(int)appendnum appendstr:(CMsg::Id)appendstr
{
	int i;
	for(i=0; itemids[i] != 0 && itemids[i] != CMsg::End; i++) {
		if (i == appendnum) {
			char label[_MAX_PATH];
			UTILITY::tcscpy(label, _MAX_PATH, CMSGV(itemids[i]));
			UTILITY::tcscat(label, _MAX_PATH, CMSGV(appendstr));
			[self addItemWithTitle:[NSString stringWithUTF8String:label]];
		} else {
			[self addItemWithTitle:[NSString stringWithUTF8String:CMSGV(itemids[i])]];
		}
	}
	if (selidx >= 0 && selidx < i) {
		[self selectItemAtIndex:selidx];
	}
}
- (void)addItemsL:(const CPtrList<CTchar> *)items selidx:(int)selidx
{
	int i;
	for(i=0; i<items->Count(); i++) {
		[self addItemWithTitle:[NSString stringWithUTF8String:items->Item(i)->GetN()]];
	}
	if (selidx >= 0 && selidx < i) {
		[self selectItemAtIndex:selidx];
	}
}
@end


/**
	Check button
*/
@implementation CocoaCheckBox
@synthesize index;
+ (CocoaCheckBox *)create:(NSRect)re title:(const char *)title action:(SEL)action value:(bool)value
{
	return [CocoaCheckBox create:re title:title index:0 action:action value:value];
}
+ (CocoaCheckBox *)create:(NSRect)re titleid:(CMsg::Id)titleid action:(SEL)action value:(bool)value
{
	return [CocoaCheckBox create:re title:CMSGV(titleid) index:0 action:action value:value];
}
+ (CocoaCheckBox *)create:(NSRect)re title:(const char *)title index:(int)index action:(SEL)action value:(bool)value
{
	CocoaCheckBox *me = [CocoaCheckBox alloc];
	[me initWithFrame:re];
	[me setTitle:[NSString stringWithUTF8String:title]];
	[me setButtonType:NSSwitchButton];
	[me setAction:action];
	[me setState:value ? NSControlStateValueOn : NSControlStateValueOff];
	me.index = index;

	return me;
}
+ (CocoaCheckBox *)create:(NSRect)re titleid:(CMsg::Id)titleid index:(int)index action:(SEL)action value:(bool)value
{
	return [CocoaCheckBox create:re title:CMSGV(titleid) index:index action:action value:value];
}
+ (CocoaCheckBox *)createT:(const char *)title action:(SEL)action value:(bool)value
{
	return [CocoaCheckBox createT:title index:0 action:action value:value];
}
+ (CocoaCheckBox *)createI:(CMsg::Id)titleid action:(SEL)action value:(bool)value
{
	return [CocoaCheckBox createT:CMSGV(titleid) index:0 action:action value:value];
}
+ (CocoaCheckBox *)createT:(const char *)title index:(int)index action:(SEL)action value:(bool)value
{
	CocoaCheckBox *me = [CocoaCheckBox alloc];
	[me init];
	[me setTitle:[NSString stringWithUTF8String:title]];
	[me setButtonType:NSSwitchButton];
	[me setAction:action];
	[me setState:value ? NSControlStateValueOn : NSControlStateValueOff];
	me.index = index;

	return me;
}
+ (CocoaCheckBox *)createI:(CMsg::Id)titleid index:(int)index action:(SEL)action value:(bool)value
{
	return [CocoaCheckBox createT:CMSGV(titleid) index:index action:action value:value];
}
+ (CocoaCheckBox *)createT:(CocoaLayout *)layout title:(const char *)title action:(SEL)action value:(bool)value
{
	CocoaCheckBox *me = [CocoaCheckBox createT:title action:action value:value];
	[layout addControl:me];
	return me;
}
+ (CocoaCheckBox *)createI:(CocoaLayout *)layout title:(CMsg::Id)titleid action:(SEL)action value:(bool)value
{
	CocoaCheckBox *me = [CocoaCheckBox createI:titleid action:action value:value];
	[layout addControl:me];
	return me;
}
+ (CocoaCheckBox *)createT:(CocoaLayout *)layout title:(const char *)title index:(int)index action:(SEL)action value:(bool)value
{
	CocoaCheckBox *me = [CocoaCheckBox createT:title index:index action:action value:value];
	[layout addControl:me];
	return me;
}
+ (CocoaCheckBox *)createI:(CocoaLayout *)layout title:(CMsg::Id)titleid index:(int)index action:(SEL)action value:(bool)value
{
	CocoaCheckBox *me = [CocoaCheckBox createI:titleid index:index action:action value:value];
	[layout addControl:me];
	return me;
}
+ (CocoaCheckBox *)createT:(CocoaLayout *)layout title:(const char *)title index:(int)index action:(SEL)action value:(bool)value width:(int)width height:(int)height
{
	CocoaCheckBox *me = [CocoaCheckBox createT:title index:index action:action value:value];
	[layout addControl:me width:width height:height];
	return me;
}
+ (CocoaCheckBox *)createI:(CocoaLayout *)layout title:(CMsg::Id)titleid index:(int)index action:(SEL)action value:(bool)value width:(int)width height:(int)height
{
	CocoaCheckBox *me = [CocoaCheckBox createI:titleid index:index action:action value:value];
	[layout addControl:me width:width height:height];
	return me;
}
@end


/**
	Radio button
*/
@implementation CocoaRadioButton
@synthesize index;
+ (CocoaRadioButton *)createT:(const char *)title action:(SEL)action value:(bool)value
{
	return [CocoaRadioButton createT:title index:0 action:action value:value];
}
+ (CocoaRadioButton *)createI:(CMsg::Id)titleid action:(SEL)action value:(bool)value
{
	return [CocoaRadioButton createT:CMSGV(titleid) index:0 action:action value:value];
}
+ (CocoaRadioButton *)createT:(const char *)title index:(int)index action:(SEL)action value:(bool)value
{
	CocoaRadioButton *me = [CocoaRadioButton alloc];
	[me init];
	[me setTitle:[NSString stringWithUTF8String:title]];
	[me setButtonType:NSRadioButton];
	[me setAction:action];
	[me setState:value ? NSControlStateValueOn : NSControlStateValueOff];
	me.index = index;

	return me;
}
+ (CocoaRadioButton *)createI:(CMsg::Id)titleid index:(int)index action:(SEL)action value:(bool)value
{
	return [CocoaRadioButton createT:CMSGV(titleid) index:index action:action value:value];
}
+ (CocoaRadioButton *)createT:(CocoaLayout *)layout title:(const char *)title action:(SEL)action value:(bool)value
{
	CocoaRadioButton *me = [CocoaRadioButton createT:title action:action value:value];
	[layout addControl:me];
	return me;
}
+ (CocoaRadioButton *)createI:(CocoaLayout *)layout title:(CMsg::Id)titleid action:(SEL)action value:(bool)value
{
	CocoaRadioButton *me = [CocoaRadioButton createI:titleid action:action value:value];
	[layout addControl:me];
	return me;
}
+ (CocoaRadioButton *)createT:(CocoaLayout *)layout title:(const char *)title index:(int)index action:(SEL)action value:(bool)value
{
	CocoaRadioButton *me = [CocoaRadioButton createT:title index:index action:action value:value];
	[layout addControl:me];
	return me;
}
+ (CocoaRadioButton *)createI:(CocoaLayout *)layout title:(CMsg::Id)titleid index:(int)index action:(SEL)action value:(bool)value
{
	CocoaRadioButton *me = [CocoaRadioButton createI:titleid index:index action:action value:value];
	[layout addControl:me];
	return me;
}
@end


/**
	Radio button group
*/
@implementation CocoaRadioGroup
+ (CocoaRadioGroup *)create:(NSRect)re rows:(int)rows cols:(int)cols titles:(const char **)titles action:(SEL)action selidx:(int)selidx
{
	CocoaRadioGroup *me = [CocoaRadioGroup alloc];
	[me initWithFrame:re mode:NSRadioModeMatrix cellClass:NSButtonCell.class numberOfRows:rows numberOfColumns:cols];
	[me setCellSize:NSMakeSize(re.size.width / cols,re.size.height / rows)];

	[me addItemsT:titles rows:rows cols:cols selidx:selidx width:-1 height:-1];

	[me setAction:action];

	return me;
}
+ (CocoaRadioGroup *)create:(NSRect)re rows:(int)rows cols:(int)cols titleids:(const CMsg::Id *)titleids action:(SEL)action selidx:(int)selidx
{
	CocoaRadioGroup *me = [CocoaRadioGroup alloc];
	[me initWithFrame:re mode:NSRadioModeMatrix cellClass:NSButtonCell.class numberOfRows:rows numberOfColumns:cols];
	[me setCellSize:NSMakeSize(re.size.width / cols,re.size.height / rows)];

	[me addItemsI:titleids rows:rows cols:cols selidx:selidx width:-1 height:-1];

	[me setAction:action];

	return me;
}

+ (CocoaRadioGroup *)create:(int)height rows:(int)rows titles:(const char **)titles action:(SEL)action selidx:(int)selidx
{
	NSRect re = NSMakeRect(0, 0, 1, height);
	return [CocoaRadioGroup create:re rows:rows cols:1 titles:titles action:action selidx:selidx];
}
+ (CocoaRadioGroup *)create:(int)height rows:(int)rows titleids:(const CMsg::Id *)titleids action:(SEL)action selidx:(int)selidx
{
	NSRect re = NSMakeRect(0, 0, 1, height);
	return [CocoaRadioGroup create:re rows:rows cols:1 titleids:titleids action:action selidx:selidx];
}
+ (CocoaRadioGroup *)create:(int)width cols:(int)cols titles:(const char **)titles action:(SEL)action selidx:(int)selidx
{
	NSRect re = NSMakeRect(0, 0, width, 1);
	return [CocoaRadioGroup create:re rows:1 cols:cols titles:titles action:action selidx:selidx];
}
+ (CocoaRadioGroup *)create:(int)width cols:(int)cols titleids:(const CMsg::Id *)titleids action:(SEL)action selidx:(int)selidx
{
	NSRect re = NSMakeRect(0, 0, width, 1);
	return [CocoaRadioGroup create:re rows:1 cols:cols titleids:titleids action:action selidx:selidx];
}
+ (CocoaRadioGroup *)create:(CocoaLayout *)layout rect:(NSRect)re rows:(int)rows cols:(int)cols titles:(const char **)titles action:(SEL)action selidx:(int)selidx
{
	CocoaRadioGroup *me = [CocoaRadioGroup create:re rows:rows cols:cols titles:titles action:action selidx:selidx];
	[layout addControl:me];
	return me;
}
+ (CocoaRadioGroup *)create:(CocoaLayout *)layout rect:(NSRect)re rows:(int)rows cols:(int)cols titleids:(const CMsg::Id *)titleids action:(SEL)action selidx:(int)selidx
{
	CocoaRadioGroup *me = [CocoaRadioGroup create:re rows:rows cols:cols titleids:titleids action:action selidx:selidx];
	[layout addControl:me];
	return me;
}
+ (CocoaRadioGroup *)create:(CocoaLayout *)layout height:(int)height rows:(int)rows titles:(const char **)titles action:(SEL)action selidx:(int)selidx
{
	CocoaRadioGroup *me = [CocoaRadioGroup create:height rows:rows titles:titles action:action selidx:selidx];
	[layout addControl:me];
	return me;
}
+ (CocoaRadioGroup *)create:(CocoaLayout *)layout height:(int)height rows:(int)rows titleids:(const CMsg::Id *)titleids action:(SEL)action selidx:(int)selidx
{
	CocoaRadioGroup *me = [CocoaRadioGroup create:height rows:rows titleids:titleids action:action selidx:selidx];
	[layout addControl:me];
	return me;
}
+ (CocoaRadioGroup *)create:(CocoaLayout *)layout width:(int)width cols:(int)cols titles:(const char **)titles action:(SEL)action selidx:(int)selidx
{
	CocoaRadioGroup *me = [CocoaRadioGroup create:width cols:cols titles:titles action:action selidx:selidx];
	[layout addControl:me];
	return me;
}
+ (CocoaRadioGroup *)create:(CocoaLayout *)layout width:(int)width cols:(int)cols titleids:(const CMsg::Id *)titleids action:(SEL)action selidx:(int)selidx
{
	CocoaRadioGroup *me = [CocoaRadioGroup create:width cols:cols titleids:titleids action:action selidx:selidx];
	[layout addControl:me];
	return me;
}
- (void)addItemsT:(const char **)titles rows:(int)rows cols:(int)cols selidx:(int)selidx width:(int)width height:(int)height
{
	int i = 0;
	int row;
	int col;
	for(row=0; row<rows && titles[i] != NULL; row++) {
		for(col=0; col<cols && titles[i] != NULL; col++) {
			NSButtonCell *cell=[self cellAtRow:row column:col];
			[cell setButtonType:NSRadioButton];
			[cell setTitle:[NSString stringWithUTF8String:titles[i]]];
			if (i == selidx) {
				[self selectCell:cell];
			}
			i++;
		}
	}
}
- (void)addItemsI:(const CMsg::Id *)titleids rows:(int)rows cols:(int)cols selidx:(int)selidx width:(int)width height:(int)height
{
	int i = 0;
	int row;
	int col;
	for(row=0; row<rows && titleids[i] != 0 && titleids[i] != CMsg::End; row++) {
		for(col=0; col<cols && titleids[i] != 0 && titleids[i] != CMsg::End; col++) {
			NSButtonCell *cell=[self cellAtRow:row column:col];
			[cell setButtonType:NSRadioButton];
			[cell setTitle:[NSString stringWithUTF8String:CMSGV(titleids[i])]];
			if (i == selidx) {
				[self selectCell:cell];
			}
			i++;
		}
	}
}
@end


/**
	Slider control
*/
@implementation CocoaSlider
@synthesize index;
#ifdef COCOA_USE_OLDSTYLE_LAYOUT
+ (CocoaSlider *)create:(NSRect)re action:(SEL)action value:(int)value
{
	return [CocoaSlider create:re index:0 action:action value:value];
}
+ (CocoaSlider *)create:(NSRect)re index:(int)index action:(SEL)action value:(int)value
{
	CocoaSlider *me = [CocoaSlider alloc];

	[me initWithFrame:re];
	[me setAction:action];
	[me setMaxValue:100.0];
	[me setMinValue:0.0];
	[me setIntValue:value];
	me.index = index;

	return me;
}
#endif

+ (CocoaSlider *)createN:(SEL)action value:(int)value
{
	return [CocoaSlider createN:0 action:action value:value];
}
+ (CocoaSlider *)createN:(SEL)action min:(int)min_val max:(int)max_val value:(int)value
{
	return [CocoaSlider createN:0 action:action min:min_val max:max_val value:value];
}
+ (CocoaSlider *)createN:(int)index action:(SEL)action value:(int)value
{
	return [CocoaSlider createN:index action:action min:0 max:100 value:value];
}
+ (CocoaSlider *)createN:(int)index action:(SEL)action min:(int)min_val max:(int)max_val value:(int)value
{
	CocoaSlider *me = [CocoaSlider alloc];
	
	[me init];
	[me setAction:action];
	[me setMaxValue:max_val];
	[me setMinValue:min_val];
	[me setIntValue:value];
	me.index = index;
	
	return me;
}
+ (CocoaSlider *)createN:(CocoaLayout *)layout action:(SEL)action value:(int)value width:(int)width height:(int)height
{
	CocoaSlider *me = [CocoaSlider createN:action value:value];
	[layout addControl:me width:width height:height];
	return me;
}
+ (CocoaSlider *)createN:(CocoaLayout *)layout action:(SEL)action min:(int)min_val max:(int)max_val value:(int)value width:(int)width height:(int)height
{
	CocoaSlider *me = [CocoaSlider createN:action min:min_val max:max_val value:value];
	[layout addControl:me width:width height:height];
	return me;
}
+ (CocoaSlider *)createN:(CocoaLayout *)layout index:(int)index action:(SEL)action value:(int)value width:(int)width height:(int)height
{
	CocoaSlider *me = [CocoaSlider createN:index action:action value:value];
	[layout addControl:me width:width height:height];
	return me;
}
+ (CocoaSlider *)createN:(CocoaLayout *)layout index:(int)index action:(SEL)action min:(int)min_val max:(int)max_val value:(int)value width:(int)width height:(int)height
{
	CocoaSlider *me = [CocoaSlider createN:index action:action min:min_val max:max_val value:value];
	[layout addControl:me width:width height:height];
	return me;
}
@end


/**
	Stepper control
*/
@implementation CocoaStepper
@synthesize text;
- (void)changeStepperValue:(CocoaStepper *)sender
{
	[text setIntValue:[self intValue]];
}
- (void)changeTextFieldValue:(CocoaTextField *)sender
{
	[self setIntValue:[text intValue]];
}
+ (CocoaStepper *)createMin:(int)min_val max:(int)max_val value:(int)value
{
	CocoaStepper *stepper = [CocoaStepper alloc];

	[stepper init];
	[stepper setMaxValue:max_val];
	[stepper setMinValue:min_val];
	[stepper setIntValue:value];
	[stepper setValueWraps:FALSE];
	[stepper setTarget:stepper];
	[stepper setAction:@selector(changeStepperValue:)];

	CocoaTextField *text = [CocoaTextField createN:value action:@selector(changeTextFieldValue:)];
	[text setTarget:stepper];
	[stepper setText:text];
	
	return stepper;
}
+ (CocoaStepper *)createN:(CocoaLayout *)layout min:(int)min_val max:(int)max_val value:(int)value
{
	CocoaStepper *step = [CocoaStepper createMin:min_val max:max_val value:value];
	[layout addControl:step.text];
	[layout addControl:step];
	return step;
}
+ (CocoaStepper *)createN:(CocoaLayout *)layout min:(int)min_val max:(int)max_val value:(int)value width:(int)width
{
	CocoaStepper *step = [CocoaStepper createMin:min_val max:max_val value:value];
	[layout addControl:step.text width:width];
	[layout addControl:step];
	return step;
}
+ (CocoaStepper *)createN:(CocoaLayout *)layout min:(int)min_val max:(int)max_val value:(int)value width:(int)width height:(int)height
{
	CocoaStepper *step = [CocoaStepper createMin:min_val max:max_val value:value];
	[layout addControl:step.text width:width height:height];
	[layout addControl:step height:height];
	return step;
}
@end


/**
	Tab view
*/
@implementation CocoaTabView
#ifdef COCOA_USE_OLDSTYLE_LAYOUT
+ (CocoaTabView *)create:(NSRect)re
{
	CocoaTabView *me = [CocoaTabView alloc];

	[me initWithFrame:re];
	[me setTabViewType:NSTopTabsBezelBorder];
	return me;
}
+ (CocoaTabView *)create:(NSRect)re tabs:(const char **)tabs
{
	CocoaTabView *me = [CocoaTabView create:re];
	[me addTabItemsT:tabs];

	return me;
}
+ (CocoaTabView *)create:(NSRect)re tabids:(const CMsg::Id *)tabids
{
	CocoaTabView *me = [CocoaTabView create:re];
	[me addTabItemsI:tabids];

	return me;
}
#endif

+ (CocoaTabView *)create
{
	CocoaTabView *me = [CocoaTabView alloc];

	[me init];
	[me setTabViewType:NSTopTabsBezelBorder];
	return me;
}
+ (CocoaTabView *)createT:(const char **)tabs
{
	CocoaTabView *me = [CocoaTabView create];
	[me addTabItemsT:tabs];

	return me;
}
+ (CocoaTabView *)createI:(const CMsg::Id *)tabids
{
	CocoaTabView *me = [CocoaTabView create];
	[me addTabItemsI:tabids];

	return me;
}
+ (CocoaTabView *)create:(CocoaLayout *)layout width:(int)width height:(int)height
{
	CocoaTabView *me = [CocoaTabView create];
	[layout addControl:me width:width height:height];
	return me;
}
+ (CocoaTabView *)createT:(CocoaLayout *)layout tabs:(const char **)tabs width:(int)width height:(int)height
{
	CocoaTabView *me = [CocoaTabView createT:tabs];
	[layout addControl:me width:width height:height];
	return me;
}
+ (CocoaTabView *)createI:(CocoaLayout *)layout tabs:(const CMsg::Id *)tabids width:(int)width height:(int)height
{
	CocoaTabView *me = [CocoaTabView createI:tabids];
	[layout addControl:me width:width height:height];
	return me;
}
- (void)addTabItemsT:(const char **)tabs
{
	int i;
	for(i=0; tabs[i] != NULL; i++) {
		[self addTabItemT:tabs[i]];
	}
}
- (void)addTabItemsI:(const CMsg::Id *)tabids
{
	int i;
	for(i=0; tabids[i] != 0 && tabids[i] != CMsg::End; i++) {
		[self addTabItemT:CMSGV(tabids[i])];
	}
}
- (NSTabViewItem *)addTabItemT:(const char *)label
{
	NSString *str = [NSString stringWithUTF8String:label];
	NSTabViewItem *item = [[NSTabViewItem alloc] init];
	CocoaView *view = [[CocoaView alloc] init];
	[item setLabel:str];
	[item setView:view];
	[self addTabViewItem:item];
	return item;
}
- (NSTabViewItem *)addTabItemI:(CMsg::Id)label_id
{
	return [self addTabItemT:CMSGV(label_id)];
}

#ifdef COCOA_USE_OLDSTYLE_LAYOUT
- (NSRect)adjustFrameSize:(NSRect)re
{
	NSUInteger i;
	int max_right = 0;
	int max_bottom = 0;
	int max_margin = 0;

	NSArray *items = [self tabViewItems];
	NSUInteger count = [items count];
	for(i=0; i < count; i++) {
		NSTabViewItem *item = [items objectAtIndex:i];
		CocoaView *view = (CocoaView *)[item view];
		if (max_right < view.max_right) max_right = view.max_right;
		if (max_bottom < view.max_bottom) max_bottom = view.max_bottom;
		if (max_margin < view.margin) max_margin = view.margin;
	}
	max_right += max_margin + 10;
	max_bottom += max_margin + 48;
	[self setFrameSize:NSMakeSize(max_right, max_bottom)];
	re.size.width = max_right;
	re.size.height = max_bottom;
	return re;
}
#endif
@end


/**
	Static surrounding box
*/
@implementation CocoaBox
#ifdef COCOA_USE_OLDSTYLE_LAYOUT
+ (CocoaBox *)create:(NSRect)re title:(const char *)title
{
	CocoaBox *me = [CocoaBox alloc];

	[me initWithFrame:re];
	CocoaView *view = [[CocoaView alloc]init];
	[me setContentView:view];
	[me setTitle:[NSString stringWithUTF8String:title]];

	return me;
}
+ (CocoaBox *)create:(NSRect)re titleid:(CMsg::Id)titleid
{
	return [CocoaBox create:re title:CMSGV(titleid)];
}
#endif

+ (CocoaBox *)createT:(const char *)title
{
	CocoaBox *me = [CocoaBox alloc];

	[me init];
	CocoaView *view = [[CocoaView alloc]init];
	[me setContentView:view];
	[me setTitle:[NSString stringWithUTF8String:title]];

	return me;
}
+ (CocoaBox *)createI:(CMsg::Id)titleid
{
	return [CocoaBox createT:CMSGV(titleid)];
}
+ (CocoaBox *)createT:(CocoaLayout *)layout :(const char *)title :(int)width :(int)height
{
	CocoaBox *me = [CocoaBox createT:title];
	[layout addControl:me width:width height:height];
	[layout setContentView:[me contentView]];
	return me;
}
+ (CocoaBox *)createI:(CocoaLayout *)layout :(CMsg::Id)titleid :(int)width :(int)height
{
	CocoaBox *me = [CocoaBox createI:titleid];
	[layout addControl:me width:width height:height];
	[layout setContentView:[me contentView]];
	return me;
}

#ifdef COCOA_USE_OLDSTYLE_LAYOUT
- (NSRect)adjustFrameSize:(NSRect)re
{
#if 1
	CocoaView *view = [self contentView];
	view.max_right += view.margin + 4;
	view.max_bottom += view.margin + 22;
	[self setFrameSize:NSMakeSize(view.max_right, view.max_bottom)];
	re.size.width = view.max_right;
	re.size.height = view.max_bottom;
#else
	[self sizeToFit];
	NSRect fre = [self frame];
	re.size.width = fre.size.width;
	re.size.height = fre.size.height;
#endif
	return re;
}
#endif
@end


/**
	Scroll view
*/
@implementation CocoaScrollView
+ (CocoaScrollView *)create:(CocoaLayout *)layout hasvs:(bool)hasvs hashs:(bool)hashs width:(int)width height:(int)height
{
	CocoaScrollView *me = [[CocoaScrollView alloc] init];

	if (hasvs) {
		NSRect re = NSMakeRect(0,0,24,height);
		NSScroller *vs = [[NSScroller alloc] initWithFrame:re];
		[me setVerticalScroller:vs];
		[me setHasVerticalScroller:YES];
	}
	if (hashs) {
		NSRect re = NSMakeRect(0,0,width,24);
		NSScroller *hs = [[NSScroller alloc] initWithFrame:re];
		[me setHorizontalScroller:hs];
		[me setHasHorizontalScroller:YES];
	}

	[layout addControl:me width:width height:height];
	return me;
}
@end


/**
	Base view
*/
@implementation CocoaView
@synthesize max_bottom;
@synthesize max_right;
@synthesize margin;
- (id)init
{
	[super init];
	//	[self setAutoresizesSubviews:YES];
	//	[self setAutoresizingMask:NSViewHeightSizable];
	[self setAutoresizesSubviews:NO];
	[self setAutoresizingMask:NSViewNotSizable];

	max_right = 0;
	max_bottom = 0;

	margin = COCOA_DEFAULT_MARGIN;

	return self;
}
- (BOOL)isFlipped
{
	return YES;
}

#ifdef COCOA_USE_OLDSTYLE_LAYOUT
- (NSRect)makeRect:(int)left :(int)top :(int)w :(int)h
{
	if (max_right < left + w) max_right = left + w;
	if (max_bottom < top + h) max_bottom = top + h;

	return NSMakeRect(left,top,w,h);
}
- (NSRect)remakeSize:(NSRect)re :(int)w :(int)h
{
	int left = re.origin.x;
	int top = re.origin.y;

	return [self makeRect:left:top:w:h];
}
- (NSRect)remakePosition:(NSRect)re :(int)left :(int)top :(int)new_w :(int)new_h
{
	return [self makeRect:left:top:new_w:new_h];
}
- (NSRect)remakeLeft:(NSRect)re :(int)left :(int)new_w :(int)new_h
{
	int top = re.origin.y;

	return [self makeRect:left:top:new_w:new_h];
}
- (NSRect)remakeLeft:(NSRect)re :(int)left
{
	return [self remakeLeft:re:left:re.size.width:re.size.height];
}
- (NSRect)remakeTop:(NSRect)re :(int)top :(int)new_w :(int)new_h
{
	int left = re.origin.x;

	return [self makeRect:left:top:new_w:new_h];
}
- (NSRect)remakeTop:(NSRect)re :(int)top
{
	return [self remakeTop:re:top:re.size.width:re.size.height];
}
- (NSRect)addPosition:(NSRect)re :(int)offsetx :(int)offsety :(int)new_w :(int)new_h
{
	int left = re.origin.x;
	int top = re.origin.y;

	return [self makeRect:left+offsetx:top+offsety:new_w:new_h];
}
- (NSRect)addWidth:(NSRect)re :(int)new_w :(int)new_h
{
	return [self addPosition:re:re.size.width:0:new_w:new_h];
}
- (NSRect)addHeight:(NSRect)re :(int)new_w :(int)new_h
{
	return [self addPosition:re:0:re.size.height:new_w:new_h];
}
+ (int)topPos:(NSRect)re
{
	return re.origin.y;
}
+ (int)bottomPos:(NSRect)re
{
	return re.origin.y + re.size.height;
}
- (void)adjustFrameSize
{
	max_right = max_right + margin;
	max_bottom = max_bottom + margin;
}
#endif

@end


/**
	Base panel
*/
@implementation CocoaBasePanel
- (id)init
{
	[super init];

	CocoaView *view = [[CocoaView alloc] init];
	[self setContentView:view];

	return self;
}
- (int)margin
{
	CocoaView *view = [self contentView];
	return view.margin;
}
#ifdef COCOA_USE_OLDSTYLE_LAYOUT
- (void)adjustFrame
{
	CocoaView *view = [self contentView];
	[view adjustFrameSize];
	NSRect now_re = NSMakeRect(0,0,view.max_right,view.max_bottom);
	now_re = [self frameRectForContentRect:now_re];
	[self setFrame:now_re display:YES];
}
#endif
- (void)setTitleById:(CMsg::Id)titleid
{
	[self setTitle:[NSString stringWithUTF8String:gMessages.Get(titleid)]];
}

@end


/**
	Layout widget controls
*/
@implementation CocoaLayoutControls
@synthesize box;
@synthesize ctrl;
@synthesize x;
@synthesize y;
@synthesize px;
@synthesize py;
@synthesize w;
@synthesize h;
@synthesize next;
@synthesize prev;
+ (CocoaLayoutControls *)create:(CocoaLayout *)box_ :(NSView *)ctrl_ :(int)x_ :(int)y_ :(int)px_ :(int)py_ :(int)w_ :(int)h_
{
	CocoaLayoutControls *me = [[CocoaLayoutControls alloc] init];

	me.box = box_;
	me.ctrl = ctrl_;
	me.x = x_;
	me.y = y_;
	me.px = px_;
	me.py = py_;
	me.w = w_;
	me.h = h_;
	me.next = nil;
	me.prev = nil;

	return me;
}
@end


/**
	Manage layouting widget controls
*/
@implementation CocoaLayout
@synthesize contentView;
@synthesize orient;
@synthesize align;
@synthesize re;
@synthesize realized;
@synthesize padding;
@synthesize margin;
@synthesize controls;
@synthesize control_nums;
+ (CocoaLayout *)create:(CocoaView *)view_ :(int)orient_
{
	CocoaLayout *me = [CocoaLayout alloc];

	[me initWith:view_ :orient_ :(LeftPos | TopPos) :0 :nil];

	return me;
}
+ (CocoaLayout *)create:(CocoaView *)view_ :(int)orient_ :(int)align_
{
	CocoaLayout *me = [CocoaLayout alloc];

	[me initWith:view_ :orient_ :align_ :0 :nil];

	return me;
}
+ (CocoaLayout *)create:(CocoaView *)view_ :(int)orient_ :(int)align_ :(int)margin_
{
	CocoaLayout *me = [CocoaLayout alloc];

	[me initWith:view_ :orient_ :align_ :margin_ :nil];

	return me;
}
+ (CocoaLayout *)create:(CocoaView *)view_ :(int)orient_ :(int)align_ :(int)margin_ :(const _TCHAR *)name_
{
	CocoaLayout *me = [CocoaLayout alloc];

	[me initWith:view_ :orient_ :align_ :margin_ :name_];

	return me;
}
- (id)initWith:(CocoaView *)view_ :(int)orient_ :(int)align_ :(int)margin_ :(const _TCHAR *)name_
{
	[super init];

	contentView = view_;
	orient = (enOrient)orient_;
	align = align_;
	margin.left = margin_;
	margin.top = margin_;
	margin.right = margin_;
	margin.bottom = margin_;
	re.x = 0;
	re.y = 0;
	re.w = 0;
	re.h = 0;

	padding = 4;

	control_nums = 0;
	controls = nil;

	realized = false;

	*name = _T('\0');
	if (name_) {
		UTILITY::tcscpy(name, sizeof(name), name_);
	}

	return self;
}
- (void)addWidth:(int)width_
{
	if (orient == HorizontalBox) {
		re.w += width_;
	} else {
		if (width_ > re.w) {
			re.w = width_;
		}
	}
}
- (void)addHeight:(int)height_
{
	if (orient == HorizontalBox) {
		if (height_ > re.h) {
			re.h = height_;
		}
	} else {
		re.h += height_;
	}
}
- (CocoaLayout *)addBox:(int)orient_
{
	CocoaLayout *box = [CocoaLayout create:nil :orient_];
	[self addItem:box];
	return box;
}
- (CocoaLayout *)addBox:(int)orient_ :(int)align_
{
	CocoaLayout *box = [CocoaLayout create:nil :orient_ :align_];
	[self addItem:box];
	return box;
}
- (CocoaLayout *)addBox:(int)orient_ :(int)align_ :(int)margin_
{
	CocoaLayout *box = [CocoaLayout create:nil :orient_ :align_ :margin_];
	[self addItem:box];
	return box;
}
- (CocoaLayout *)addBox:(int)orient_ :(int)align_ :(int)margin_ :(const _TCHAR *)name_
{
	CocoaLayout *box = [CocoaLayout create:nil :orient_ :align_ :margin_ :name_];
	[self addItem:box];
	return box;
}
- (void)addControl:(NSControl *)ctrl_
{
	[ctrl_ sizeToFit];
	NSRect now_re = [ctrl_ frame];
	[self addItem:nil :ctrl_ :now_re.size.width :now_re.size.height :0 :0];
}
- (void)addControl:(NSControl *)ctrl_ width:(int)width_
{
	[ctrl_ sizeToFit];
	NSRect now_re = [ctrl_ frame];
	if (now_re.size.width < width_) {
		now_re.size.width = width_;
	}
	[self addItem:nil :ctrl_ :now_re.size.width :now_re.size.height :0 :0];
}
- (void)addControl:(NSControl *)ctrl_ height:(int)height_
{
	[ctrl_ sizeToFit];
	NSRect now_re = [ctrl_ frame];
	if (now_re.size.height < height_) {
		now_re.size.height = height_;
	}
	[self addItem:nil :ctrl_ :now_re.size.width :now_re.size.height :0 :0];
}
- (void)addControl:(NSView *)ctrl_ width:(int)width_ height:(int)height_
{
	[self addItem:nil :ctrl_ :width_ :height_ :0 :0];
}
- (void)addControl:(NSView *)ctrl_ width:(int)width_ height:(int)height_ x:(int)px_ y:(int)py_
{
	[self addItem:nil :ctrl_ :width_ :height_ :px_ :py_];
}
- (void)addControlWithBox:(CocoaLayout *)box_ :(NSView *)ctrl_
{
	[self addItem:box_ :ctrl_ :0 :0 :0 :0];
}
- (void)addSpace:(int)width_ :(int)height_
{
	[self addItem:nil :0 :width_ :height_ :0 :0];
}
- (void)addSpace:(int)width_ :(int)height_ :(int)px_ :(int)py_
{
	[self addItem:nil :0 :width_ :height_ :px_ :py_];
}
- (void)addItem:(CocoaLayout *)box_
{
	[self addItem:box_ :nil :0 :0 :0 :0];
}
- (void)addItem:(CocoaLayout *)box_ :(NSView *)ctrl_ :(int)width_ :(int)height_ :(int)px_ :(int)py_
{
	CocoaLayoutControls *newitem = [CocoaLayoutControls create:box_ :ctrl_ :-1 :-1 :px_ :py_ :width_ :height_];

	CocoaLayoutControls *item = controls;
	if (controls != nil) {
		// next item
		while (item.next != nil) {
			item = item.next;
		}
		item.next = newitem;
		newitem.prev = item;
	} else {
		// first item
		controls = newitem;
	}
	control_nums++;
	
	item = newitem;

	if (box_ != nil) {
		[box_ setContentView: [self contentView]];
	}
	if (ctrl_ != nil) {
		[[self contentView] addSubview:ctrl_];
	}
}
- (int)maxHeight:(CocoaLayoutControls *)item_
{
	int maxh = 0;
	while(item_ != nil) {
		if (item_.ctrl && maxh < item_.h) {
			maxh = item_.h;
		}
		item_ = item_.next;
	}
	return maxh;
}
- (void)realize:(NSPanel *)dlg_
{
	if (realized) return;

	[self realizeReal:dlg_];
	[self moveItems:nil:dlg_];

	NSRect now_re = [[dlg_ contentView] frame];
//	now_re.origin.x += COCOA_DEFAULT_MARGIN;
//	now_re.origin.y += COCOA_DEFAULT_MARGIN;
	now_re.size.width = re.w;
	now_re.size.height = re.h;
//	[[dlg_ contentView] setFrame:now_re];

//	now_re.origin.x = 0;
//	now_re.origin.y = 0;
	now_re = [dlg_ frameRectForContentRect:now_re];
	[dlg_ setFrame:now_re display:YES];
}
- (void)realizeReal:(NSPanel *)dlg_
{
	if (realized) return;

	// add left top margin
	re.w += margin.left;
	re.h += margin.top;

#ifdef _DEBUG_CBOX
	logging->out_debugf(_T("Realize: %-8s IN x:%d y:%d w:%d h:%d  margin:l:%d r:%d t:%d b:%d"), name
					, re.x, re.y, re.w, re.h
					, margin.left, margin.right, margin.top, margin.bottom);
#endif
	int maxh = [self maxHeight:controls];
	int mw,mh;
	int nums = 0;
	CocoaLayoutControls *item = controls;
	while(item != nil) {
		if (item.box != nil) {
			// box
			switch(self.orient) {
			case VerticalBox:
				if (nums > 0) {
					re.h += padding;
				}
				[item.box adjustPosition:re.x + margin.left:re.y + re.h];
				item.x = margin.left;
				item.y = re.h;
				break;
			case HorizontalBox:
				if (nums > 0) {
					re.w += padding;
				}
				[item.box adjustPosition:re.x + re.w:re.y + margin.top];
				item.x = re.w;
				item.y = margin.top;
				break;
			case TabViewBox:
				[item.box adjustPosition:margin.left:margin.top];
				item.x = margin.left;
				item.y = margin.top;
				break;
			case BoxViewBox:
				[item.box adjustPosition:margin.left:margin.top];
				item.x = margin.left;
				item.y = margin.top;
				break;
			}

			[item.box realizeReal:dlg_];

			item.w = [item.box getWidthWithMargin];
			item.h = [item.box getHeightWithMargin];

			switch(self.orient) {
			case VerticalBox:
				re.h += item.h;
				mw = margin.left + item.w;
				if (re.w < mw) {
					re.w = mw;
				}
				break;
			case HorizontalBox:
				re.w += item.w;
				mh = margin.top + item.h;
				if (re.h < mh) {
					re.h = mh;
				}
				break;
			case TabViewBox:
				mw = margin.left + item.w;
				if (re.w < mw) {
					re.w = mw;
				}
				mh = margin.top + item.h;
				if (re.h < mh) {
					re.h = mh;
				}
				break;
			case BoxViewBox:
				mw = margin.left + item.w;
				if (re.w < mw) {
					re.w = mw;
				}
				mh = margin.top + item.h;
				if (re.h < mh) {
					re.h = mh;
				}
				break;
			}

#ifdef _DEBUG_CBOX
			logging->out_debugf(_T("Realize: %-8s B%d re:x:%d y:%d w:%d h:%d item:x:%d y:%d w:%d h:%d"), name, nums
							, re.x, re.y, re.w, re.h
							, item.x, item.y, item.w, item.h);
#endif

		} else {
			// control or space
			switch(self.orient) {
			case VerticalBox:
				if (nums > 0) {
					re.h += padding;
				}
				item.x = margin.left;
				item.y = re.h;
				re.h += item.h;
				mw = margin.left + item.w;
				if (re.w < mw) {
					re.w = mw;
				}
				break;
			case HorizontalBox:
				if (nums > 0) {
					re.w += padding;
				}
				item.x = re.w;
				item.y = margin.top + (maxh - item.h) / 2;
				re.w += item.w;
				mh = margin.top + item.h;
				if (re.h < mh) {
					re.h = mh;
				}
				break;
			case TabViewBox:
				item.x = margin.left;
				item.y = margin.top;
				mw = margin.left + item.w;
				if (re.w < mw) {
					re.w = mw;
				}
				mh = margin.top + item.h;
				if (re.h < mh) {
					re.h = mh;
				}
				break;
			case BoxViewBox:
				item.x = margin.left;
				item.y = margin.top;
				mw = margin.left + item.w;
				if (re.w < mw) {
					re.w = mw;
				}
				mh = margin.top + item.h;
				if (re.h < mh) {
					re.h = mh;
				}
				break;
			}
#ifdef _DEBUG_CBOX
			logging->out_debugf(_T("Realize: %-8s C%d re:x:%d y:%d w:%d h:%d item:x:%d y:%d w:%d h:%d"), name, nums
							, re.x, re.y, re.w, re.h
							, item.x, item.y, item.w, item.h);
#endif
		}
		item = item.next;
		nums++;
	}

	// add right bottom margin
	re.w += margin.right;
	re.h += margin.bottom;

	if (self.orient == TabViewBox) {
		// set max width and height for children
		item = controls;
		while(item != nil) {
			if (item.box != nil) {
				switch(item.box.orient) {
					case VerticalBox:
						item.w = re.w - margin.left - margin.right;
						[item.box setWidth:item.w];
						break;
					case HorizontalBox:
						item.h = re.h - margin.top - margin.bottom;
						[item.box setHeight:item.h];
						break;
					default:
						break;
				}
			}
#ifdef _DEBUG_CBOX
			logging->out_debugf(_T("Realize: %-8s TabView Max item:x:%d y:%d w:%d h:%d"), name
							, item.x, item.y, item.w, item.h);
#endif
			item = item.next;
		}
	}


	if (self.orient == TabViewBox) {
		NSString *cname = [controls.ctrl className];
		if ([cname isEqualToString:@"CocoaTabView"] || [cname isEqualToString:@"NSTabView"]) {
			NSTabView *tab = (NSTabView *)controls.ctrl;
			NSRect tre = [tab contentRect];
#ifdef _DEBUG_CBOX
			logging->out_debugf(_T("Realize: %-8s TabView co:x:%d y:%d w:%d h:%d re:x:%d y:%d w:%d h:%d item:x:%d y:%d w:%d h:%d"), name
							, (int)tre.origin.x, (int)tre.origin.y, (int)tre.size.width, (int)tre.size.height
							, re.x, re.y, re.w, re.h
							, controls.x, controls.y, controls.w, controls.h);
#endif
			re.w += (int)tre.origin.x * 2;
			re.h += (int)tre.origin.y + (int)tre.origin.x;
		}
	} else if (self.orient == BoxViewBox) {
		NSString *cname = [controls.ctrl className];
		if ([cname isEqualToString:@"CocoaBox"] || [cname isEqualToString:@"NSBox"]) {
			NSBox *box = (NSBox *)controls.ctrl;
			NSRect tre = [[box contentView] frame];
			tre.size.width = re.w;
			tre.size.height = re.h;
			[box setFrameFromContentFrame:tre];
			tre = [box frame];
			re.w = tre.size.width;
			re.h = tre.size.height;
		}
	}

#ifdef _DEBUG_CBOX
	logging->out_debugf(_T("Realize: %-8s re x:%d y:%d w:%d h:%d"), name, re.x, re.y, re.w, re.h);
#endif

	realized = true;
}

- (void)adjustPosition:(int)x_ :(int)y_
{
	re.x = x_;
	re.y = y_;
}

- (void)moveItems:(CocoaLayout *)parent :(NSPanel *)dlg_
{
	if (!realized) return;

	if (parent) {
		switch(self.orient) {
		case VerticalBox:
			if ((align & 0xf0) == BottomPos) {
				re.y = [parent getHeightWithTopMargin] - re.h;
			} else if ((align & 0xf0) == MiddlePos) {
				re.y = ([parent getHeightWithTopMargin] - re.h) / 2;
			}
			break;
		case HorizontalBox:
			if ((align & 0x0f) == RightPos) {
				re.x = [parent getWidthWithLeftMargin] - re.w;
			} else if ((align & 0x0f) == CenterPos) {
				re.x = ([parent getWidthWithLeftMargin] - re.w) / 2;
			}
			break;
		case TabViewBox:
			if (controls.w < re.w) {
				controls.w = re.w;
			}
			if (controls.h < re.h) {
				controls.h = re.h;
			}
			break;
		case BoxViewBox:
			if (controls.w < re.w) {
				controls.w = re.w;
			}
			if (controls.h < re.h) {
				controls.h = re.h;
			}
			break;
		}
	}

#ifdef _DEBUG_CBOX
	int nums = 0;
#endif
	CocoaLayoutControls *item = controls;
	while(item != nil) {
		if (item.box != nil) {
			[item.box moveItems:self :dlg_];
#ifdef _DEBUG_CBOX
			logging->out_debugf(_T("MoveItems: %-8s B%d re:x:%d y:%d w:%d h:%d item:x:%d y:%d w:%d h:%d"), name, nums
							, re.x, re.y, re.w, re.h
							, item.x, item.y, item.w, item.h);
#endif
		} 
		if (item.ctrl != nil) {

			switch(self.orient) {
			case VerticalBox:
				if ((align & 0x0f) == CenterPos) {
					item.x = (re.w - item.w) / 2;
				}
				break;
			case HorizontalBox:
				if ((align & 0xf0) == MiddlePos) {
					item.y = (re.h - item.h) / 2;
				}
				break;
			default:
				break;
			}

			NSRect new_re;
			new_re = NSMakeRect(re.x + item.x + item.px, re.y + item.y + item.py, item.w - item.px, item.h - item.py);
			[item.ctrl setFrame:new_re];

#ifdef _DEBUG_CBOX
			logging->out_debugf(_T("MoveItems: %-8s C%d re:x:%d y:%d w:%d h:%d item:x:%d y:%d w:%d h:%d"), name, nums
							, re.x, re.y, re.w, re.h
							, item.x, item.y, item.w, item.h);
#endif
		}
		item = item.next;

#ifdef _DEBUG_CBOX
		nums++;
#endif
	}
#ifdef _DEBUG_CBOX
	logging->out_debugf(_T("MoveItems: %-8s re re:x:%d y:%d w:%d h:%d item:x:%d y:%d w:%d h:%d"), name
					, re.x, re.y, re.w, re.h
					, re.x, re.y, re.w, re.h);
#endif
}

- (int)getWidthWithMargin
{
	return re.w;
}

- (int)getHeightWithMargin
{
	return re.h;
}

- (int)getWidthWithLeftMargin
{
	return re.w - margin.right;
}

- (int)getHeightWithTopMargin
{
	return re.h - margin.bottom;
}

- (int)getWidth
{
	return re.w - margin.left - margin.right;
}

- (int)getHeight
{
	return re.h - margin.right - margin.bottom;
}

- (void)setWidth:(int)width
{
	re.w = width;
}

- (void)setHeight:(int)height
{
	re.h = height;
}

- (void)getPositionByItem:(int)num_ :(int *)x_ :(int *)y_
{
	int nums = 0;
	CocoaLayoutControls *item = controls;
	while(item != nil) {
		if (num_ == nums) {
			*x_ = re.x + item.x + item.px;
			*y_ = re.y + item.y + item.py;
			break;
		}
		item = item.next;
		nums++;
	}
}

- (void)setLeftMargin:(int)val
{
	margin.left = val;
}

- (void)setTopMargin:(int)val
{
	margin.top = val;
}

@end
