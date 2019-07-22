﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "UTcpCommunicatorv2.h"
#include "TcpClientv.h"
#include "Engine.h"
#include "JsonUtilities.h"
#include "Json.h"
#include "MyBlueprintFunctionLibrary.h"
class TcpClientv* UUTcpCommunicatorv2::mtcp = nullptr;
void UUTcpCommunicatorv2::clientexit()
{
	FString outstring;
	FMessagePackage messagepackage;
	messagepackage.MT = MessageType::EXITGAME;
	FJsonObjectConverter::UStructToJsonObjectString<FMessagePackage>(messagepackage, outstring);
	UUTcpCommunicatorv2::mtcp->Send(outstring);
	FPlatformProcess::Sleep(0.03);
	UMyBlueprintFunctionLibrary::CLogtofile(FString("OnGameEndevent.Broadcast();"));
	UUTcpCommunicatorv2::mtcp->exitthread = true;
}
void UUTcpCommunicatorv2::init()
{
	UMyBlueprintFunctionLibrary::AddfunctiontoOnGameexitArray(&UUTcpCommunicatorv2::clientexit);

}
void UUTcpCommunicatorv2::OnTcpResponse(const TArray<uint8>&p, const FString & str)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, *str);
	FMessagePackage mp;
	FJsonObjectConverter::JsonObjectStringToUStruct<FMessagePackage>(str, &mp, 0, 0);
	if (mp.MT == MessageType::EntryMAP)
	{
		FString pld = mp.PayLoad;

		FString param = FString::Printf(TEXT("?%s?%s"), *LevelShouldBeLoaded, TEXT("hiparam2"));
		pld.Append(param);
		UGameplayStatics::OpenLevel(world, *pld);
		//UGameplayStatics::OpenLevel(GetWorld(), "192.168.1.240:7788");
	}
}

void UUTcpCommunicatorv2::OpenServermap(FString name)
{
	FMessagePackage messagepackage(MessageType::MATCH, FString("hi"));
	LevelShouldBeLoaded = name;
	if (mtcp)
	{	
		FString outstring;
		messagepackage.MT = MessageType::EXITGAME;
		FJsonObjectConverter::UStructToJsonObjectString<FMessagePackage>(messagepackage, outstring);
		mtcp->Send(outstring);
		UMyBlueprintFunctionLibrary::CLogtofile(FString("OnGameEndevent.Broadcast();"));
		mtcp->exitthread = true;
		FPlatformProcess::Sleep(0.03);
		mtcp = nullptr;
	}
	mtcp = new TcpClientv();

		FString outstring;
		messagepackage.MT = MessageType::MATCH;
		messagepackage.PayLoad = LevelShouldBeLoaded;
		FJsonObjectConverter::UStructToJsonObjectString<FMessagePackage>(messagepackage, outstring);
		mtcp->Connecttoserver(192, 168, 1, 240, 8001);
		mtcp->OnTcpClientReceiveddata.AddDynamic(this, &UUTcpCommunicatorv2::OnTcpResponse);
		mtcp->Send(outstring);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, *outstring);
}