// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Spline.generated.h"

UCLASS()
class DEMOPROJECT_API ASpline : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpline();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void AddMesh(int index);
	USplineComponent* m_pSpline;
	UStaticMesh* m_pMesh;
	FTimerHandle m_TimerHandle;
	float m_MinDistanceBetweenSplinePoints{30.f};
	float m_MinDistanceBetweenActorPoints{ 50.f };
	TArray<FVector> m_ActorLocations{};

	void AddSplinePoint();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void StartDrawing();

	UFUNCTION(BlueprintCallable)
	void StopDrawing();

	UFUNCTION(BlueprintCallable)
	TArray<FVector> GetPoinstAlongSpline(int amountOfActors);
};
