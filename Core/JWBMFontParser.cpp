#include "JWBMFontParser.h"

using namespace tinyxml2;
using namespace JWEngine;

// Static member variable declaration
BMFont JWBMFontParser::ms_FontData;

auto JWBMFontParser::ParseComma(const STRING& Data, UINT ID) noexcept->UINT
{
	UINT Result = 0;
	STRING tempString = Data;
	tempString += ',';

	int Count = 0;
	size_t FoundPrev = 0;
	size_t Found = 0;

	while (true)
	{
		Found = tempString.find_first_of(',', FoundPrev);
		if (ID == Count)
			break;

		FoundPrev = Found + 1;
		Count++;
	}

	tempString = tempString.substr(FoundPrev, Found - FoundPrev);
	Result = atoi(tempString.c_str());

	return Result;
}

auto JWBMFontParser::IsParsed() const noexcept->bool
{
	return ms_FontData.IsFontDataParsed;
}

auto JWBMFontParser::Parse(const WSTRING& FileName) noexcept->bool
{
	if (IsParsed())
	{
		// Avoid duplicate parsing
		//@warning: at this point Parse() returns 'false'(== parsing failed)
		return false;
	}

	// Clear ms_FontData
	ms_FontData.vPages.clear();
	ms_FontData.vChars.clear();
	ms_FontData.vKernings.clear();
	ms_FontData.CharMap.clear();
	ms_FontData.KerningMap.clear();
	memset(ms_FontData.MappedCharacters, 0, KMaxWcharIndex);

	//@warning: Without "tinyxml2::" here, XMLDocument can be ambiguous because of <msxml.h> in Windows Kits
	tinyxml2::XMLDocument tempXMLDoc;
	
	if (tempXMLDoc.LoadFile(WstringToString(FileName).c_str()) == tinyxml2::XMLError::XML_SUCCESS)
	{
		const XMLElement* tempElementRoot = tempXMLDoc.FirstChildElement("font");

		const XMLElement* tempElement = nullptr;
		const XMLAttribute* tempAttr = nullptr;

		/**
		* Parse element <info>
		*/
		tempElement = tempElementRoot->FirstChildElement("info");
		tempAttr = tempElement->FirstAttribute();
		ms_FontData.Info.Face = StringToWstring(tempAttr->Value());
		tempAttr = tempAttr->Next();
		ms_FontData.Info.Size = tempAttr->UnsignedValue();
		tempAttr = tempAttr->Next();
		ms_FontData.Info.IsBold = tempAttr->BoolValue();
		tempAttr = tempAttr->Next();
		ms_FontData.Info.IsItalic = tempAttr->BoolValue();
		tempAttr = tempAttr->Next();
		ms_FontData.Info.Charset = StringToWstring(tempAttr->Value());
		tempAttr = tempAttr->Next();
		ms_FontData.Info.IsUnicode = tempAttr->BoolValue();
		tempAttr = tempAttr->Next();
		ms_FontData.Info.StretchH = tempAttr->UnsignedValue();
		tempAttr = tempAttr->Next();
		ms_FontData.Info.IsSmooth = tempAttr->BoolValue();
		tempAttr = tempAttr->Next();
		ms_FontData.Info.AA = tempAttr->UnsignedValue();
		tempAttr = tempAttr->Next();

		ms_FontData.Info.Padding.Up = ParseComma(tempAttr->Value(), 0);
		ms_FontData.Info.Padding.Right = ParseComma(tempAttr->Value(), 1);
		ms_FontData.Info.Padding.Down = ParseComma(tempAttr->Value(), 2);
		ms_FontData.Info.Padding.Left = ParseComma(tempAttr->Value(), 3);
		tempAttr = tempAttr->Next();

		ms_FontData.Info.Spacing.Horz = ParseComma(tempAttr->Value(), 0);
		ms_FontData.Info.Spacing.Vert = ParseComma(tempAttr->Value(), 1);
		tempAttr = tempAttr->Next();

		ms_FontData.Info.bOutline = tempAttr->UnsignedValue();

		/**
		* Parse element <common>
		*/
		tempElement = tempElementRoot->FirstChildElement("common");
		tempAttr = tempElement->FirstAttribute();
		ms_FontData.Common.LineHeight = tempAttr->UnsignedValue();
		tempAttr = tempAttr->Next();
		ms_FontData.Common.Base = tempAttr->IntValue();
		tempAttr = tempAttr->Next();
		ms_FontData.Common.ScaleW = tempAttr->UnsignedValue();
		tempAttr = tempAttr->Next();
		ms_FontData.Common.ScaleH = tempAttr->UnsignedValue();
		tempAttr = tempAttr->Next();
		ms_FontData.Common.Pages = tempAttr->UnsignedValue();
		tempAttr = tempAttr->Next();
		ms_FontData.Common.IsPacked = tempAttr->BoolValue();
		tempAttr = tempAttr->Next();
		ms_FontData.Common.AlphaChnl = tempAttr->IntValue();
		tempAttr = tempAttr->Next();
		ms_FontData.Common.RedChnl = tempAttr->IntValue();
		tempAttr = tempAttr->Next();
		ms_FontData.Common.GreenChnl = tempAttr->IntValue();
		tempAttr = tempAttr->Next();
		ms_FontData.Common.BlueChnl = tempAttr->IntValue();

		/**
		* Parse element <pages>
		*/
		tempElement = tempElementRoot->FirstChildElement("pages");
		tempElement = tempElement->FirstChildElement("page");

		for (UINT i = 0; i < ms_FontData.Common.Pages; i++)
		{
			BMFont::BMPage tempPage;
			tempAttr = tempElement->FirstAttribute();
			tempPage.ID = tempAttr->UnsignedValue();
			tempPage.File = StringToWstring(tempAttr->Next()->Value());
			ms_FontData.vPages.push_back(tempPage);

			tempElement = tempElement->NextSiblingElement();
		}

		/**
		* Parse element <chars>
		*/
		tempElement = tempElementRoot->FirstChildElement("chars");
		UINT tempElementCount = tempElement->FirstAttribute()->IntValue();
		tempElement = tempElement->FirstChildElement("char");

		for (UINT i = 0; i < tempElementCount; i++)
		{
			BMFont::BMChar tempChar;
			tempAttr = tempElement->FirstAttribute();
			tempChar.ID = tempAttr->UnsignedValue();
			tempAttr = tempAttr->Next();
			tempChar.X = tempAttr->UnsignedValue();
			tempAttr = tempAttr->Next();
			tempChar.Y = tempAttr->UnsignedValue();
			tempAttr = tempAttr->Next();
			tempChar.Width = tempAttr->UnsignedValue();
			tempAttr = tempAttr->Next();
			tempChar.Height = tempAttr->UnsignedValue();
			tempAttr = tempAttr->Next();
			tempChar.XOffset = tempAttr->IntValue();
			tempAttr = tempAttr->Next();
			tempChar.YOffset = tempAttr->IntValue();
			tempAttr = tempAttr->Next();
			tempChar.XAdvance = tempAttr->UnsignedValue();
			tempAttr = tempAttr->Next();
			tempChar.Page = tempAttr->UnsignedValue();
			tempAttr = tempAttr->Next();
			tempChar.Chnl = tempAttr->UnsignedValue();

			tempChar.X_f = static_cast<float>(tempChar.X);
			tempChar.Y_f = static_cast<float>(tempChar.Y);
			tempChar.Width_f = static_cast<float>(tempChar.Width);
			tempChar.Height_f = static_cast<float>(tempChar.Height);
			tempChar.XOffset_f = static_cast<float>(tempChar.XOffset);
			tempChar.YOffset_f = static_cast<float>(tempChar.YOffset);
			tempChar.XAdvance_f = static_cast<float>(tempChar.XAdvance);

			// Save tempChar to vChars
			ms_FontData.vChars.push_back(tempChar);

			// map Chars
			ms_FontData.CharMap.insert(std::make_pair(static_cast<wchar_t>(tempChar.ID), ms_FontData.vChars.size() - 1));

			tempElement = tempElement->NextSiblingElement();
		}

		/**
		* Parse element <kernings>
		*/
		//
		//@warning: It's possible that there isn't any kerning in the file!
		//
		tempElement = tempElementRoot->FirstChildElement("kernings");
		if (tempElement)
		{
			tempElementCount = tempElement->FirstAttribute()->IntValue();
			tempElement = tempElement->FirstChildElement("kerning");

			for (UINT i = 0; i < tempElementCount; i++)
			{
				BMFont::BMKerning tempKerning;
				tempAttr = tempElement->FirstAttribute();
				tempKerning.First = tempAttr->UnsignedValue();
				tempAttr = tempAttr->Next();
				tempKerning.Second = tempAttr->UnsignedValue();
				tempAttr = tempAttr->Next();
				tempKerning.Amount = tempAttr->IntValue();
				ms_FontData.vKernings.push_back(tempKerning);

				// Map Kernings
				ms_FontData.KerningMap.insert(std::make_pair(std::make_pair(tempKerning.First, tempKerning.Second), tempKerning.Amount));

				tempElement = tempElement->NextSiblingElement();
			}
		}

		// Map all possible characters from 0 to KMaxWcharIndex
		wchar_t wchar_t_value = 0;
		size_t Chars_ID = 0;
		for (size_t iterator_wchar_t = 0; iterator_wchar_t < KMaxWcharIndex; iterator_wchar_t++)
		{
			wchar_t_value = static_cast<wchar_t>(iterator_wchar_t);

			// Find Chars_ID in CharMap
			auto iterator_line_character = ms_FontData.CharMap.find(wchar_t_value);
			if (iterator_line_character != ms_FontData.CharMap.end())
			{
				// Set Chars_ID value only if the key exists
				Chars_ID = iterator_line_character->second;
			}
			else
			{
				Chars_ID = 0;
			}

			ms_FontData.MappedCharacters[iterator_wchar_t] = Chars_ID;
		}

		// The parsing ended successfully
		ms_FontData.IsFontDataParsed = true;

		return true;
	}

	//@warning: If failed at LoadFile(), the method directly comes here
	return false;
}

auto JWBMFontParser::GetFontTextureWidth() const noexcept->float
{
	return static_cast<float>(ms_FontData.Common.ScaleW);
}

auto JWBMFontParser::GetFontTextureHeight() const noexcept->float
{
	return static_cast<float>(ms_FontData.Common.ScaleH);
}

auto JWBMFontParser::GetBMCharFromWideCharacter(wchar_t WideCharacter) const noexcept->BMFont::BMChar&
{
	return ms_FontData.vChars[GetCharsIDFromWideCharacter(WideCharacter)];
}

PRIVATE auto JWBMFontParser::GetCharsIDFromWideCharacter(wchar_t WideCharacter) const noexcept->size_t
{
	WideCharacter = static_cast<wchar_t>(min(WideCharacter, KMaxWcharIndex));

	return ms_FontData.MappedCharacters[WideCharacter];
}