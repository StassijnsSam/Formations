// Fill out your copyright notice in the Description page of Project Settings.


#include "FollowTarget.h"

// Sets default values
AFollowTarget::AFollowTarget()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFollowTarget::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFollowTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

