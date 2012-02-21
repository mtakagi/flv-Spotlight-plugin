#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h> 

/* -----------------------------------------------------------------------------
   Step 1
   Set the UTI types the importer supports
  
   Modify the CFBundleDocumentTypes entry in Info.plist to contain
   an array of Uniform Type Identifiers (UTI) for the LSItemContentTypes 
   that your importer can handle
  
   ----------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
   Step 2 
   Implement the GetMetadataForURL function
  
   Implement the GetMetadataForURL function below to scrape the relevant
   metadata from your document and return it as a CFDictionary using standard keys
   (defined in MDItem.h) whenever possible.
   ----------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
   Step 3 (optional) 
   If you have defined new attributes, update schema.xml and schema.strings files
   
   The schema.xml should be added whenever you need attributes displayed in 
   Finder's get info panel, or when you have custom attributes.  
   The schema.strings should be added whenever you have custom attributes. 
 
   Edit the schema.xml file to include the metadata keys that your importer returns.
   Add them to the <allattrs> and <displayattrs> elements.
  
   Add any custom types that your importer requires to the <attributes> element
  
   <attribute name="com_mycompany_metadatakey" type="CFString" multivalued="true"/>
  
   ----------------------------------------------------------------------------- */



/* -----------------------------------------------------------------------------
    Get metadata attributes from file
   
   This function's job is to extract useful information your file format supports
   and return it as a dictionary
   ----------------------------------------------------------------------------- */

#define kFLVSignature "FLV"
#define kFLVMediaTypeSound CFSTR("Sound")
#define kFLVMediaTypeVideo CFSTR("Video")
#define kFLVVersionAttributeKey CFSTR("com_adobe_flash_video_version")
#define kFLVOnMetaDataCanSeekToEndAttributeKey CFSTR("com_adobe_flash_video_onMetaData_canSeekToEnd")
#define kFLVOnMetaDataFrameRateAttributeKey CFSTR("com_adobe_flash_video_onMetaData_framerate")
#define kFLVOnMetaDataAudioDelayAttributeKey CFSTR("com_adobe_flash_video_onMetaData_Audio_Delay")

#define kFLVOnMetaDataString "onMetaData"

enum ScriptDataType {
	AMF_Number = 0,
	AMF_Boolean,
	AMF_String,
	AMF_Object,
	AMF_Movie_Clip, // reserved, not supported
	AMF_Null,
	AMF_Undefined,
	AMF_Reference,
	AMF_ECMA_Array,
	AMF_Object_End_Marker,
	AMF_Strict_Array,
	AMF_Date,
	AMF_Long_String,
};

enum SoundFormat {
	LinearPCMPlatform = 0,
	ASPCM,
	MP3,
	LinearPCMLittle,
	Nellymoser16kMono,
	Nellymoser8kMono,
	Nellymoser,
	G711AlawLogarithmicPCM, // reserved
	G711mulawLogarithmicPCM, // reserved
	AAC = 10, // AAC is supported in Flash Player 9,0,115,0 and higher.
	Speex = 11, // Speex is supported in Flash Player 10 and higher.
	MP38k = 14, // reserved
	DeviceSpecificSound
};

enum FrameType {
	KeyFrame = 1,
	InterFrame,
	DisposableInterFrame,
	GeneratedKeyFrame,
	VideoInfoCommandFrame,
};

enum VideoCodec {
	SorensonH263 = 2,
	ScreenVideo,
	On2VP6,
	On2VP6WithAlphaChannel,
	ScreenVideoV2,
	AVC, 
};

void ParseScriptData(CFReadStreamRef stream, CFMutableDictionaryRef attributes);

Boolean IsFLVFile(CFReadStreamRef stream)
{
	char *signature = calloc(4, sizeof(char));
	CFIndex returnVal;
	Boolean status = FALSE;
	
	returnVal = CFReadStreamRead(stream, (UInt8 *)signature, 3);
	
	if (returnVal == 0 || returnVal == -1) return FALSE;
	if (strncmp(signature, kFLVSignature, 4) == 0) status = TRUE;
	
	if (!status) fprintf(stderr, "Error file is not flv file %x%x%x\n", signature[0],signature[1],signature[2]);
	
	free(signature);
	
	return status;
}

CFStringRef GetAudioCodecString(enum SoundFormat format)
{
	CFStringRef codec;
	
	switch (format) {
		case LinearPCMPlatform: codec = CFSTR("Linear PCM, platform endian");
			break;
		case ASPCM: codec = CFSTR("ADPCM");
			break;
		case MP3: codec = CFSTR("MP3");
			break;
		case LinearPCMLittle: codec = CFSTR("Linear PCM, little endian");
			break;
		case Nellymoser16kMono: codec = CFSTR("Nellymoser 16 kHz mono");
			break;
		case Nellymoser8kMono: codec = CFSTR("Nellymoser 8 kHz mono");
			break;
		case Nellymoser: codec = CFSTR("Nellymoser");
			break;
		case G711AlawLogarithmicPCM: codec = CFSTR("G.711 A-law logarithmic PCM");
			break;
		case G711mulawLogarithmicPCM: codec = CFSTR("G.711 mu-law logarithmic PCM");
			break;
		case AAC: codec = CFSTR("AAC");
			break;
		case Speex: codec = CFSTR("Speex"); 
			break;
		case MP38k: codec = CFSTR("MP3 8 kHz");
			break;
		case DeviceSpecificSound: codec = CFSTR("Device-specific sound");
			break;
		default: codec = CFSTR("Unknown sound format");
	}
	
	return codec;
}

CFStringRef GetVideoCodecString(enum VideoCodec codecID)
{
	CFStringRef codec;
	
	switch (codecID) {
		case SorensonH263: codec = CFSTR("Sorenson H.263");
			break;
		case ScreenVideo: codec = CFSTR("Screen video");
			break;
		case On2VP6: codec = CFSTR("On2 VP6");
			break;
		case On2VP6WithAlphaChannel: codec = CFSTR("On2 VP6 with alpha channel");
			break;
		case ScreenVideoV2: codec = CFSTR("Screen video version 2");
			break;
		case AVC: codec = CFSTR("AVC");
			break; 
	}
	
	return codec;
}

//void ParseAudioTag(NSData *data)
//{
//	uint8_t firstByte[1];
//	uint8_t soundFormat;
//	uint8_t soundRate;
//	uint8_t soundSize;
//	uint8_t soundType;
//	char *format;
//	char *rate;
//	char *size;
//	char *type;
//	
//	[data getBytes:(void *)firstByte range:NSMakeRange(position, 1)];
//	soundFormat = (firstByte[0] & 0xF0) >> 4;
//	soundRate = (firstByte[0] & 0xC) >> 2;
//	soundSize = (firstByte[0] & 0x2) >> 1;
//	soundType = firstByte[0] & 0x1;
//	
//	//	NSLog(@"%u", firstByte[1]);
//	switch (soundFormat) {
//		case LinearPCMPlatform: format = "Linear PCM, platform endian";
//			break;
//		case ASPCM: format = "ADPCM";
//			break;
//		case MP3: format = "MP3";
//			break;
//		case LinearPCMLittle: format = "Linear PCM, little endian";
//			break;
//		case Nellymoser16kMono: format = "Nellymoser 16 kHz mono";
//			break;
//		case Nellymoser8kMono: format = "Nellymoser 8 kHz mono";
//			break;
//		case Nellymoser: format = "Nellymoser";
//			break;
//		case G711AlawLogarithmicPCM: format = "G.711 A-law logarithmic PCM";
//			break;
//		case G711mulawLogarithmicPCM: format = "G.711 mu-law logarithmic PCM";
//			break;
//		case AAC: format = "AAC";
//			break;
//		case Speex: format = "Speex"; 
//			break;
//		case MP38k: format = "MP3 8 kHz";
//			break;
//		case DeviceSpecificSound: format = "Device-specific sound";
//		default: format = "Unknown sound format";
//	}
//	
//	switch (soundRate) {
//		case 0: rate = "5.5 kHz";
//			break;
//		case 1: rate = "11 kHz";
//			break;
//		case 2: rate = "22 kHz";
//			break;
//		case 3: rate = "44 kHz";
//			break;
//	}
//	
//	switch (soundSize) {
//		case 0: size = "8-bit samples";
//			break;
//		case 1: size = "16-bit samples";
//			break;
//	}
//	
//	switch (soundType) {
//		case 0: type = "Mono sound";
//			break;
//		case 1: type = "Stereo sound";
//	}
//	
//	NSLog(@"Sound format: %s Sound rate: %s Sound size: %s Sound type: %s", format, rate, size, type);
//}
//
//void ParseVideoTag(NSData *data)
//{
//	uint8_t firstByte[1];
//	uint8_t frameType;
//	uint8_t codecID;
//	char *frame;
//	char *codec;
//	
//	[data getBytes:(void *)firstByte range:NSMakeRange(position, 1)];
//	frameType = (firstByte[0] & 0xF0) >> 4;
//	codecID = firstByte[0] & 0xF;
//	
//	switch (frameType) {
//		case KeyFrame: frame = "key frame (for AVC, a seekable frame)";
//			break;
//		case InterFrame: frame = "inter frame (for AVC, a non-seekable frame)";
//			break;
//		case DisposableInterFrame: frame = "disposable inter frame (H.263 only)";
//			break;
//		case GeneratedKeyFrame: frame = "generated key frame (reserved for server use only)";
//			break;
//		case VideoInfoCommandFrame: frame = "video info/command frame";
//	}
//	
//	switch (codecID) {
//		case SorensonH263: codec = "Sorenson H.263";
//			break;
//		case ScreenVideo: codec = "Screen video";
//			break;
//		case On2VP6: codec = "On2 VP6";
//			break;
//		case On2VP6WithAlphaChannel: codec = "On2 VP6 with alpha channel";
//			break;
//		case ScreenVideoV2: codec = "Screen video version 2";
//			break;
//		case AVC: codec = "AVC";
//			break; 
//	}
//	
//	NSLog(@"Frame type: %s Codec ID: %s", frame, codec);
//}
//void ParseECMAArray(NSData *data, long position);

// http://d.hatena.ne.jp/h0shu/20071115/p1
// http://ja.wikipedia.org/wiki/IEEE_754
static double GetDoubleFromIEE754(UInt8 *buffer)
{
	uint8_t sign = (buffer[0] & 0x80) ? -1 : 1;
	uint16_t exp = ((buffer[0] & 0x7F) << 4) | (buffer[1] >> 4);
	uint64_t mantissa = ((uint64_t)(buffer[1] & 0xF) << 48L) | ((uint64_t)buffer[2] << 40L) | ((uint64_t)buffer[3] << 36L) | ((uint64_t)buffer[4] << 28L) | ((uint64_t)buffer[5] << 20L) | ((uint64_t)buffer[6] << 12L) | (uint64_t)buffer[7];
	
	//	NSLog(@"%u", sign);
	//	NSLog(@"%u", exp);
	//	NSLog(@"%llu", (uint64_t)(buffer[1] & 0x7f) << 48L);
	//	NSLog(@"%llu", mantissa);
	
	if (mantissa == 0 && exp == 0) return .0f;
	
	return sign * pow(2.0f, exp - 1023) * ((mantissa | 0x10000000000000) / 4503599627370496.f);
}

double GetNumber(CFReadStreamRef stream)
{
	UInt8 number[8];
	
	CFReadStreamRead(stream, number, 8);
	
	return GetDoubleFromIEE754(number);
}

Boolean GetBoolean(CFReadStreamRef stream)
{
	UInt8 boolean[1];
	CFReadStreamRead(stream, boolean, 1);
	
	return (boolean[0] ? TRUE : FALSE);
}

CFStringRef CFStringCreateFromAMFString(CFReadStreamRef stream)
{
	UInt8 length[2];
	UInt8 *string;
	CFIndex index;
	CFStringRef key;
	CFReadStreamRead(stream, length, 2);
	
	
	if ((length[0] == 0) && (length[1] == 0)) return NULL;
	
	index = (length[0] << 8) | length[1];
	fprintf(stderr, "String length: %lu\n", index);
	string = calloc(index + 1, sizeof(UInt8));
	CFReadStreamRead(stream, string, index);
	fprintf(stderr, "String: %s\n", (char *)string);
	key = CFStringCreateWithBytes(kCFAllocatorDefault, string, index, kCFStringEncodingUTF8, TRUE);
	
	free(string);
	
	return key;
}

void SetAttributeFromStreamWithKey(CFReadStreamRef stream, CFStringRef key, CFMutableDictionaryRef attributes)
{	
	UInt8 dummy[1];
	CFReadStreamRead(stream, dummy, 1);
//	fprintf(stderr, "%u\n", dummy[0]);
	
	if (CFEqual(key, CFSTR("audiocodecid"))) {
		int audioCodecID = (int)GetNumber(stream);
		CFStringRef codec = GetAudioCodecString(audioCodecID);
		CFArrayRef codecs = NULL;
		
		if (CFDictionaryGetValueIfPresent(attributes, kMDItemCodecs, (void *)&codecs)) {
			CFMutableArrayRef array = CFArrayCreateMutableCopy(kCFAllocatorDefault, 2, codecs);
			CFArrayAppendValue(array, codec);
			CFDictionaryAddValue(attributes, kMDItemCodecs, array);
			CFRelease(array);
		} else {
			codecs = CFArrayCreate(kCFAllocatorDefault, (void *)codec, 1, &kCFTypeArrayCallBacks);
			CFDictionaryAddValue(attributes, kMDItemCodecs, codecs);
			CFRelease(codecs);
		}
	} else if (CFEqual(key, CFSTR("audiodatarate"))) {
		double rate = GetNumber(stream);
		CFNumberRef bitrate = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &rate);
		
		CFDictionaryAddValue(attributes, kMDItemAudioBitRate, bitrate);
		CFRelease(bitrate);
	} else if (CFEqual(key, CFSTR("audiodelay"))) {
		double delay = GetNumber(stream);
		CFNumberRef audioDelay = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &delay);
		
		CFDictionaryAddValue(attributes, kFLVOnMetaDataAudioDelayAttributeKey, audioDelay);
		CFRelease(audioDelay);
	} else if (CFEqual(key, CFSTR("audiosamplerate"))) {
		double rate = GetNumber(stream);
		CFNumberRef samplerate = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &rate);
		
		CFDictionaryAddValue(attributes, kMDItemAudioSampleRate, samplerate);
		CFRelease(samplerate);
	} else if (CFEqual(key, CFSTR("audiosamplesize"))) {
		double size = GetNumber(stream);
		CFNumberRef samplesize = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &size);
		
		CFDictionaryAddValue(attributes, kMDItemBitsPerSample, samplesize);
		CFRelease(samplesize);
	} else if (CFEqual(key, CFSTR("canSeekToEnd"))) {
		CFBooleanRef canSeekToEnd = GetBoolean(stream) ? kCFBooleanTrue : kCFBooleanFalse;
		CFDictionaryAddValue(attributes, kFLVOnMetaDataCanSeekToEndAttributeKey, canSeekToEnd);
	} else if (CFEqual(key, CFSTR("creationdate"))) {
		
	} else if (CFEqual(key, CFSTR("duration"))) {
		double duration = GetNumber(stream);
		CFNumberRef durationSeconds = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &duration);
		
		CFDictionaryAddValue(attributes, kMDItemDurationSeconds, durationSeconds);
		CFRelease(durationSeconds);
	} else if (CFEqual(key, CFSTR("filesize"))) {
		
	} else if (CFEqual(key, CFSTR("framerate"))) {
		double rate = GetNumber(stream);
		CFNumberRef framerate = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &rate);
		
		CFDictionaryAddValue(attributes, kFLVOnMetaDataFrameRateAttributeKey, framerate);
		CFRelease(framerate);
	} else if (CFEqual(key, CFSTR("height"))) {
		double height = GetNumber(stream);
		CFNumberRef pixelHeight = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &height);
		
		CFDictionaryAddValue(attributes, kMDItemPixelHeight, pixelHeight);
		CFRelease(pixelHeight);
	} else if (CFEqual(key, CFSTR("stereo"))) {
		int count = GetBoolean(stream) ? 2 : 1;
		CFNumberRef channelCount = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &count);
		
		CFDictionaryAddValue(attributes, kMDItemAudioChannelCount, channelCount);
		CFRelease(channelCount);
	} else if (CFEqual(key, CFSTR("videocodecid"))) {
		int codecID = (int)GetNumber(stream);
		CFStringRef codec = GetVideoCodecString(codecID);
		CFArrayRef codecs = NULL;
		
		if (CFDictionaryGetValueIfPresent(attributes, kMDItemCodecs, (void *)codecs)) {
			CFMutableArrayRef array = CFArrayCreateMutableCopy(kCFAllocatorDefault, 2, codecs);
			CFArrayAppendValue(array, codec);
			CFDictionaryAddValue(attributes, kMDItemCodecs, array);
			CFRelease(array);
		} else {
			codecs = CFArrayCreate(kCFAllocatorDefault, (void *)codec, 1, &kCFTypeArrayCallBacks);
			CFDictionaryAddValue(attributes, kMDItemCodecs, codecs);
			CFRelease(codecs);
		}
	} else if (CFEqual(key, CFSTR("videodatarate"))) {
		double rate = GetNumber(stream);
		CFNumberRef bitrate = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &rate);
		
		CFDictionaryAddValue(attributes, kMDItemVideoBitRate, bitrate);
		CFRelease(bitrate);
	} else if (CFEqual(key, CFSTR("width"))) {
		double width = GetNumber(stream);
		CFNumberRef pixelWidth = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &width);
		
		CFDictionaryAddValue(attributes, kMDItemPixelWidth, pixelWidth);
		CFRelease(pixelWidth);
	}
}

static void PrintProperty(CFReadStreamRef stream, CFMutableDictionaryRef attributes)
{
	CFStringRef key = CFStringCreateFromAMFString(stream);
	
	assert(key);
	
	if ((CFEqual(key, CFSTR("audiocodecid")) ||
		CFEqual(key, CFSTR("audiodatarate")) ||
		CFEqual(key, CFSTR("audiodelay")) ||
		CFEqual(key, CFSTR("audiosamplerate")) ||
		CFEqual(key, CFSTR("audiosamplesize")) ||
		CFEqual(key, CFSTR("canSeekToEnd")) ||
//		CFEqual(key, CFSTR("creationdate")) ||
		CFEqual(key, CFSTR("duration")) ||
//		CFEqual(key, CFSTR("filesize")) ||
		CFEqual(key, CFSTR("framerate")) ||
		CFEqual(key, CFSTR("height")) ||
		CFEqual(key, CFSTR("stereo")) ||
		CFEqual(key, CFSTR("videocodecid")) ||
		CFEqual(key, CFSTR("videodatarate")) ||
		CFEqual(key, CFSTR("width")))){
		SetAttributeFromStreamWithKey(stream, key, attributes);
	} else {
		ParseScriptData(stream, attributes);
	}
	
	if (key) CFRelease(key);
}

void ParseECMAArray(CFReadStreamRef stream, CFMutableDictionaryRef attributes)
{
	uint32_t length[1];
	CFReadStreamRead(stream, (UInt8 *)length, 4);
	length[0] = CFSwapInt32HostToBig(length[0]);

	while(length[0]--) {
		PrintProperty(stream, attributes);
	}
	ParseScriptData(stream, attributes);
}

void ParseStrictArray(CFReadStreamRef stream, CFMutableDictionaryRef attributes)
{
	uint32_t length[1];
	UInt8 marker[3];
	
	CFReadStreamRead(stream, (UInt8 *)length, 4);
	length[0] = CFSwapInt32HostToBig(length[0]);
	
	fprintf(stderr, "Strict Array Length: %u\n", length[0]);
	
	while(length[0]--) {
		ParseScriptData(stream, attributes);
	}
	
	CFReadStreamRead(stream, marker, 3);
	
	if (!((marker[0] == 0) && (marker[1] == 0) && (marker[2] == 9))) {
		CFNumberRef offset = CFReadStreamCopyProperty(stream, kCFStreamPropertyFileCurrentOffset);
		SInt64 currentOffset;
		CFNumberGetValue(offset, kCFNumberSInt64Type, &currentOffset);
		currentOffset -= 3;
		CFRelease(offset);
		offset = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &currentOffset);
//		CFShow(offset);
		CFReadStreamSetProperty(stream, kCFStreamPropertyFileCurrentOffset, offset);
		CFRelease(offset);
	}
}

void ParseDate(CFReadStreamRef stream, CFMutableDictionaryRef attributes)
{
	UInt8 date[8];
	SInt16 localDateTimeOffset[1];
	double dateTime[1];
	
	CFReadStreamRead(stream, date, 8);
	CFReadStreamRead(stream, (UInt8 *)localDateTimeOffset, 2);
	dateTime[0] = GetDoubleFromIEE754(date);
	fprintf(stderr, "Date type: %s\nLocal date offset: %d\n", ctime((time_t *)dateTime), localDateTimeOffset[0]);
}

void ParseScriptData(CFReadStreamRef stream, CFMutableDictionaryRef attributes)
{
	UInt8 type[1];
	CFReadStreamRead(stream, type, 1);
		
	switch (type[0]) {
		case AMF_Number: fprintf(stderr, "Number type: %f\n", GetNumber(stream));
			break;
		case AMF_Boolean: fprintf(stderr, "Boolean type: %s\n", (GetBoolean(stream) ? "True" : "False")) ;
			break;
		case AMF_String: {
			CFStringRef string = CFStringCreateFromAMFString(stream);

			if (CFEqual(string, CFSTR(kFLVOnMetaDataString))) {
				ParseScriptData(stream, attributes);
			}
			
			if (string) CFRelease(string);
			break;
		}
		case AMF_Object: 
		case AMF_Movie_Clip: // reserved, not supported
		case AMF_Null: 
		case AMF_Undefined:
		case AMF_Reference:
			break;
		case AMF_ECMA_Array: ParseECMAArray(stream, attributes);
			break;
		case AMF_Object_End_Marker: fprintf(stderr, "Object End Marker\n");
			break;
		case AMF_Strict_Array: ParseStrictArray(stream, attributes);
			break;
		case AMF_Date: ParseDate(stream, attributes);
			break;
		case AMF_Long_String:
		default: fprintf(stderr, "%u\n", type[0]);
	}
}

void ParseFLVTag(CFReadStreamRef stream, CFMutableDictionaryRef attributes, Boolean *isAudioTagParsed, Boolean *isVideoTagParsed)
{
	UInt8 buffer[15];
	uint32_t size;
	CFReadStreamRead(stream, buffer, 15);
	
	size = ((buffer[5] << 16) | (buffer[6] << 8) | buffer[7]);
	
	switch (buffer[4] & 0x1F) {
		case 0x08: // Tag type: Audio tag
			*isAudioTagParsed = TRUE;
			break;
		case 0x09: // Tag type: Video tag
			*isVideoTagParsed = TRUE;
			break;
		case 0x12: // Tag type: Script data
			fprintf(stderr, "Parse SCRIPT DATA\n");
			ParseScriptData(stream, attributes);
			*isAudioTagParsed = TRUE;
			*isVideoTagParsed = TRUE;
			break;
	}
}


Boolean GetMetadataForURL(void* thisInterface, 
			   CFMutableDictionaryRef attributes, 
			   CFStringRef contentTypeUTI,
			   CFURLRef urlForFile)
{
    /* Pull any available metadata from the file at the specified path */
    /* Return the attribute keys and attribute values in the dict */
    /* Return TRUE if successful, FALSE if there was no data provided */
    CFReadStreamRef stream = CFReadStreamCreateWithFile(kCFAllocatorDefault, urlForFile);
	int ver;
	UInt8 buffer[6];
	CFMutableArrayRef mediaTypes;
	Boolean isAudioTagParsed = TRUE;
	Boolean isVideoTagParsed = TRUE;
	
	if (!stream) return FALSE;
	
	CFReadStreamOpen(stream);
	
	if (!IsFLVFile(stream)) {
		if (stream) CFRelease(stream);
		return FALSE;
	}
	CFReadStreamRead(stream, buffer, 6);
	mediaTypes = CFArrayCreateMutable(kCFAllocatorDefault, 2, &kCFTypeArrayCallBacks);
	
	if (buffer[1] & 0x4) {
		CFArrayAppendValue(mediaTypes, kFLVMediaTypeSound);
		isAudioTagParsed = FALSE;
	}
	
	if (buffer[1] & 0x1) {
		CFArrayAppendValue(mediaTypes, kFLVMediaTypeVideo);
		isVideoTagParsed = FALSE;
	}
	
	while(!(isAudioTagParsed && isVideoTagParsed)) {
		fprintf(stderr, "Parse flv tags\n");
		ParseFLVTag(stream, attributes, &isAudioTagParsed, &isVideoTagParsed);
	}
	
	ver = buffer[0];
//	fprintf(stderr, "Version is %d\n", *(&buffer[0]));
	CFDictionaryAddValue(attributes, kMDItemMediaTypes, mediaTypes);
	CFNumberRef version = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &ver);
	CFDictionaryAddValue(attributes, kFLVVersionAttributeKey, version);
	CFRelease(version);
	CFRelease(mediaTypes);
	CFRelease(stream);
	return TRUE;
}
