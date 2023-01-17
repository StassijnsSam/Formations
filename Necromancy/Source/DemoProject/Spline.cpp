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

	m_pRoot = CreateDefaultSubobject<USceneComponent>("Root");
	if (m_pRoot)
	{
		SetRootComponent(m_pRoot);
	}

	m_pSpline->AttachTo(m_pRoot);
	
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
	splineMesh->RegisterComponentWithWorld(GetWorld());

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
	
	//Update the previous splineMesh endtangent
	auto children = m_pSpline->GetAttachChildren();
	if (children.Num() > 0) {
		auto lastMesh = Cast<USplineMeshComponent>(children.Last());
		lastMesh->SetEndTangent(startTangent, true);
	}
	//Attach the current splinemesh
	splineMesh->AttachToComponent(m_pSpline, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
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
	const float minSplineLength{ 180.f };
	const float minDistanceToCloseLoop{ 170.f };
	//Clear the timer to add more points
	GetWorldTimerManager().ClearTimer(m_TimerHandle);
	//Already calculate the leader position
	CalculateLeaderLocation();
	//Set the root to correct location
	this->SetActorLocation(m_LeaderLocation);
	m_pSpline->SetRelativeLocation(-m_LeaderLocation);

	//Check if the start and the end point are very close together and the spline is long enough, if so, make it a closed loop
	if (m_pSpline->GetSplineLength() > minSplineLength &&
		FVector::Distance(m_pSpline->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World),
		m_pSpline->GetLocationAtSplinePoint(m_pSpline->GetNumberOfSplinePoints(), ESplineCoordinateSpace::World)) < minDistanceToCloseLoop) {
		m_pSpline->SetClosedLoop(true, true);
		//Update the last splinemesh to connect it to the start
		auto children = m_pSpline->GetAttachChildren();
		if (children.Num() > 0) {
			auto lastMesh = Cast<USplineMeshComponent>(children.Last());
			auto firstmesh = Cast<USplineMeshComponent>(children[0]);
			lastMesh->SetEndTangent(firstmesh->GetStartTangent(), true);
			lastMesh->SetEndPosition(firstmesh->GetStartPosition(), true);
		}
	}
}

TArray<FVector> ASpline::GetPoinstAlongSpline(int amountOfActors)
{
	m_ActorLocations.Empty();
	float minDistanceBetween{ 120.f };

	//If the amount of actors is invalid or 0 just return the leaderlocation
	if (amountOfActors <= 0) {
		m_ActorLocations.Add(m_LeaderLocation);
		return m_ActorLocations;
	}

	//Get the original spline length (not affected by scale) -> If you draw very small, it will be reactive depending on the amount of actors selected
	float totalLength = m_pSpline->GetSplineLength();
	float distanceBetween = totalLength / float(amountOfActors - 1);
	if (m_pSpline->IsClosedLoop()) {
		//So the start and end dont coincide
		distanceBetween = totalLength / float(amountOfActors);
		minDistanceBetween = 150.f;
	}

	if (distanceBetween < minDistanceBetween) {
		float scaleFactor = minDistanceBetween * float(amountOfActors - 1) / totalLength;
		if (m_pSpline->IsClosedLoop()) {
			//So the start and end dont coincide
			scaleFactor = minDistanceBetween * float(amountOfActors) / totalLength;
		}
		FVector currentScale = m_pSpline->GetRelativeScale3D();
		this->SetActorScale3D(currentScale * scaleFactor);

		//redraw the splineMeshes
		RedrawSplineMeshes();
	}

	float currentDistance{};
	for (int i{}; i < amountOfActors; ++i) {
		FVector currentLocation = m_pSpline->GetLocationAtDistanceAlongSpline(currentDistance, ESplineCoordinateSpace::World);
		m_ActorLocations.Add(currentLocation);
		currentDistance += distanceBetween;
	}

	return m_ActorLocations;
}

FVector ASpline::GetLeaderLocation()
{
	CalculateLeaderLocation();
	return m_LeaderLocation;
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

void ASpline::CalculateLeaderLocation()
{
	//To get the middle we need the min and max location in x and y
	FVector min{};
	FVector max{};
	auto curveVector = m_pSpline->GetSplinePointsPosition();
	curveVector.CalcBounds(min, max);

	m_LeaderLocation = (min + max) / 2.f;
}

void ASpline::RedrawSplineMeshes()
{
	if (m_pSpline->GetNumberOfSplinePoints() <= 0) {
		return;
	}

	//delete the current spline meshes
	auto children = m_pSpline->GetAttachChildren();
	for (auto pChild : children) {
		pChild->DestroyComponent();
	}

	for (int index{ 1 }; index <= m_pSpline->GetNumberOfSplinePoints(); ++index) {
		//If the mesh is not set, return
		if (!m_pMesh) {
			return;
		}

		USplineMeshComponent* splineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
		splineMesh->SetStaticMesh(m_pMesh);
		splineMesh->SetMaterial(0, m_pMesh->GetMaterial(0));
		splineMesh->SetMobility(EComponentMobility::Movable);
		splineMesh->RegisterComponentWithWorld(GetWorld());

		const FVector startPoint = m_pSpline->GetLocationAtSplinePoint(index - 1, ESplineCoordinateSpace::World);
		const FVector startTangent = m_pSpline->GetTangentAtSplinePoint(index - 1, ESplineCoordinateSpace::World);

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
		//Attach the current splinemesh
		splineMesh->AttachToComponent(m_pSpline, FAttachmentTransformRules::KeepWorldTransform);
	}
	//If it is a closed loop, fully close it
	if (m_pSpline->IsClosedLoop()) {
		auto lastMesh = Cast<USplineMeshComponent>(children.Last());
		auto firstmesh = Cast<USplineMeshComponent>(children[0]);
		lastMesh->SetEndTangent(firstmesh->GetStartTangent(), true);
		lastMesh->SetEndPosition(firstmesh->GetStartPosition(), true);
	}
}

