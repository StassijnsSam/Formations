// Fill out your copyright notice in the Description page of Project Settings.
#include "Spline.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"

// Sets default values
ASpline::ASpline()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	m_pSpline = CreateDefaultSubobject<USplineComponent>("Spline");
	m_pSpline->ClearSplinePoints();
	m_pSpline->bDrawDebug = true;
	if (m_pSpline)
	{
		SetRootComponent(m_pSpline);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT("StaticMesh'/Game/Meshes/Cylinder.Cylinder'"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface>Material(TEXT("MaterialInstanceConstant'/Game/Materials/MI_Spline.MI_Spline'"));
	m_pMesh = MeshAsset.Object;
	m_pMesh->SetMaterial(0, Material.Object);
}

// Called when the game starts or when spawned
void ASpline::BeginPlay()
{
	Super::BeginPlay();
}

void ASpline::AddMesh(int index)
{
	//If the index is 0, you cant make a mesh yet since you need two points
	if (index == 0) {
		return;
	}
	//If the mesh is not set, return
	if (!m_pMesh) {
		return;
	}
	
	USplineMeshComponent* splineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
	splineMesh->SetStaticMesh(m_pMesh);
	splineMesh->SetMaterial(0, m_pMesh->GetMaterial(0));
	splineMesh->SetMobility(EComponentMobility::Movable);
	splineMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	splineMesh->RegisterComponentWithWorld(GetWorld());
	splineMesh->AttachToComponent(m_pSpline, FAttachmentTransformRules::KeepRelativeTransform);

	const FVector startPoint = m_pSpline->GetLocationAtSplinePoint(index -1, ESplineCoordinateSpace::World);
	const FVector startTangent = m_pSpline->GetTangentAtSplinePoint(index -1, ESplineCoordinateSpace::World);

	const FVector endPoint = m_pSpline->GetLocationAtSplinePoint(index, ESplineCoordinateSpace::World);
	const FVector endTangent = m_pSpline->GetTangentAtSplinePoint(index, ESplineCoordinateSpace::World);

	splineMesh->SetStartPosition(startPoint, true);
	splineMesh->SetStartTangent(startTangent, true);
	splineMesh->SetEndPosition(endPoint, true);
	splineMesh->SetEndTangent(endTangent, true);
	//turn of collision
	splineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//set forward access
	splineMesh->SetForwardAxis(ESplineMeshAxis::X, true);
}

// Called every frame
void ASpline::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASpline::StartDrawing()
{
	GetWorldTimerManager().SetTimer(m_TimerHandle, this, &ASpline::AddSplinePoint, 0.05f, true, 0.0f);
}

void ASpline::StopDrawing()
{
	GetWorldTimerManager().ClearTimer(m_TimerHandle);
}

TArray<FVector> ASpline::GetPoinstAlongSpline(int amountOfActors)
{
	float totalLength = m_pSpline->GetSplineLength();
	float distanceBetween = totalLength / float(amountOfActors - 1);

	float currentDistance{};
	for (int i{}; i < amountOfActors; ++i) {
		FVector currentLocation = m_pSpline->GetLocationAtDistanceAlongSpline(currentDistance, ESplineCoordinateSpace::World);
		m_ActorLocations.Add(currentLocation);
		currentDistance += distanceBetween;
	}

	return m_ActorLocations;
}

void ASpline::AddSplinePoint()
{
	auto playerController = GetWorld()->GetFirstPlayerController();
	FHitResult hit{};
	//Using custom trace channel for floor only
	bool hasHit = playerController->GetHitResultUnderCursor(ECollisionChannel::ECC_GameTraceChannel1, true, hit);

	if (hasHit) {
		//Check if the mouse is far enough from previous location
		FVector previousPointLocation = m_pSpline->GetLocationAtSplinePoint(m_pSpline->GetNumberOfSplinePoints(), ESplineCoordinateSpace::World);
		if (FVector::Distance(previousPointLocation, hit.Location) > m_MinDistanceBetweenSplinePoints) {
			m_pSpline->AddSplinePoint(hit.Location, ESplineCoordinateSpace::World, true);
			//update the mesh
			AddMesh(m_pSpline->GetNumberOfSplinePoints() - 1);
		}
	}
}

