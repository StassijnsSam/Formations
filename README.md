# Drawable Formations

Research project in Unreal Engine 4.27 about real time drawable AI Formations for Gameplay Programming. Integrated into a small RTS-style Necromancy game.

## Result
![](https://github.com/StassijnsSam/Formations/blob/main/GIFs/FullGame2.gif)

## Initial project
This research is integrated into a small RTS-style project, made with Blueprints in Unreal Engine 4.27. You are a Necromancer and can raise dead bodies as zombies. You then control these zombies by clicking on a location, enemy, or yourself.
The initial project itself includes a small tutorial.
### Controls
**WASD** - Move the Necromancer  
**R** - *"Raise"* a dead body as a zombie  
**T** - *"Trade life"*  When an enemy is selected use some of your health to send a ray that does 100 damage.  
**T** - *"Trade life"* When a zombie is selected, absorb the magic in the zombie to heal, the zombie gets destroyed  
**Right mouse** - Click and hold to use RTS style selection  
**Left mouse** - Click to target  

## Focus
During this project, I initially wanted to focus on adding a couple of formations, programming flocking and group movement with A* pathfinding on a navmesh.  
While I was researching,  I discovered that formations on their own are not as interesting as I thought, since you have to add all the different shapes manually.  
That is why I decided to shift my focus and I chose to create a way to draw any formation you want in realtime.  
There is not much research on drawable formations. So I believed it would be quite interesting to explore this subject.  

# Research
This part will discuss the challenges I faced and how I solved them.

## How to draw a formation in real time?
In Unreal Engine there are different ways to "paint" by using shaders, but I need an actual path that I can save in order to perform the necessary calculations.

I landed on using splines.  
Essentially a spline is a collection of points in the world that get connected.

I added an input key. As long as the player is pressing 'F' (for Formation), he/she will be able to draw.    
Once the player starts drawing, a Timer will start running. This Timer will add a new point every 0.1 seconds and the Timer handle is saved so it can be stopped later.   

```
void ASpline::StartDrawing()
{
	GetWorldTimerManager().SetTimer(m_TimerHandle, this, &ASpline::AddSplinePoint, 0.1f, true, 0.0f);
}
```

To add a spline point, we check where the cursor currently is and project the cursor down to get the location in the world.  
I also added a custom trace channel so only hits with the floor will be taken into account. By doing so, I could make sure the spline will be drawn on the floor of the level.  
I added a minimum distance between spline points, so if you dont move much the spline will not be made out of a bunch of points that are grouped up together.

```
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
		}
	}
}

```

When you let go of the 'F' button, the Timer will be stopped.  

```
void ASpline::StopDrawing()
{
	GetWorldTimerManager().ClearTimer(m_TimerHandle);
}
```

This code already allows to draw the spline in real time, but I also wanted to make sure the spline is visible ingame. That is where my splinemesh components come in.  
Every time a new point gets added, the function Addmesh will now be called with the index of the point that was currently added!

In order for the full splinemesh to work correctly, it is vital that both the start and end location and tangents are set. Since we draw the pieces one by one,
it is essential we update the previous segment's end tangent when a new segment is drawn.  
After that we attach the component to the spline.

```
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
	//set forward access so the mesh orientation is correct
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
```
![](https://github.com/StassijnsSam/Formations/blob/main/GIFs/Draw.gif)

This concludes the first part.  
At this point, we have a spline that can be drawn in real time as long as you hold a button and spline mesh components will be added one by one making the spline visible.  

Now we move on to the next part, where we want to use this spline to create formations.

## How to turn a spline into a formation?
The basics of this question are very simple.  
In Unreal Engine there are several functions on spline components that can help us with calculations.

We get the distance between points and use a function on the spline to get the location at a certain distance along the spline. This way we get all the points we need!  
```
TArray<FVector> ASpline::GetPoinstAlongSpline(int amountOfActors)
{
	//Clear the actorlocations
  	m_ActorLocations.Empty();

	//If the amount of actors is invalid or 0 just return empty
	if (amountOfActors <= 0) {
		return m_ActorLocations;
	}

	//Get the original spline length
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
```

In essence we now have a formation. We tell the spline how many units we currently have, and we get the points back where those actors can be placed in the formation.  
But here are the issues.

### Closed formations
Sometimes you want to draw a formation that is closed, like a circle or a square. Currently, the formation does not get closed and this gives us problems when trying to fit units on it.  
This is a big problem because both the start and endpoint will be given as locations to fit an actor, but if you drew a closed formations these points will overlap.

![](https://github.com/StassijnsSam/Formations/blob/main/GIFs/Line.gif)
![](https://github.com/StassijnsSam/Formations/blob/main/GIFs/SquareBad.gif)

To fix this, I added a check when you stop drawing to see if your formation is closed or not.  
We do this by checking the distance between the first and the last point, while also making sure the formation itself is long enough.  
This ensures that if you draw a small line for example, it will not be closed up. Additionally, we update the spline meshes so the loop also visually closes nicely.  

```
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
```

Now when getting our points, we can check if our spline is closed, and change the calculations accordingly!  

```
float distanceBetween = totalLength / float(amountOfActors - 1);
	if (m_pSpline->IsClosedLoop()) {
		//So the start and end dont coincide
		distanceBetween = totalLength / float(amountOfActors);
	}
```

![](https://github.com/StassijnsSam/Formations/blob/main/GIFs/SquareGood.gif)

This fixes our issue with closed formations.

### Formation size
I wanted to give the player full freedom. This means that any formation being drawn will be used regardless of size.  
When a formation is very big, that is not a problem. But when it is too small to fit all the units, the units will bunch up and try to push each other out of the way.  
To make this fully work I want to scale formations that are too small untill all the units can fit.  

In essence this is again very simple. We calculate the needed scalefactor and apply it to the spline. This will warp our meshes though, so we will redraw them so they still look normal.  
It is also important that the origin of the spline is set in the middle (more on this later).
```
if (distanceBetween < minDistanceBetween) {
		float scaleFactor = minDistanceBetween * float(amountOfActors - 1) / totalLength;
		if (m_pSpline->IsClosedLoop()) {
			//So the start and end dont coincide
			scaleFactor = minDistanceBetween * float(amountOfActors) / totalLength;
		}
		FVector currentScale = m_pSpline->GetRelativeScale3D();
		this->SetActorScale3D(currentScale * scaleFactor);

		//redraw the splineMeshes so they look normal
		RedrawSplineMeshes();
	}
  ```

![](https://github.com/StassijnsSam/Formations/blob/main/GIFs/Scale.gif)

This concludes the second part.  
We turned our spline into an actual formation. This formation will work with both closed and not closed splines and will scale up when the drawn spline is too small.  

## Where to put the formation leader?
Every formation has a leader that is the pivot of the formation. The choice of leader is very important for the movement of the formation, especially for rotations.  
The options we can choose from are a random unit, a unit that is actually important or a virtual leader.

I opted for a virtual leader. This means the leader is not actually a unit but the center of the formation.  
Later on this can also be used to make it look like the important unit is the leader by giving them the same location.  

![](https://github.com/StassijnsSam/Formations/blob/main/GIFs/Leader.gif)

We can again use a spline function to help us calculate the location of our virtual leader.  
  ```
void ASpline::CalculateLeaderLocation()
{
	//To get the middle we need the min and max location in x and y
	FVector min{};
	FVector max{};
	auto curveVector = m_pSpline->GetSplinePointsPosition();
	curveVector.CalcBounds(min, max);

	m_LeaderLocation = (min + max) / 2.f;
}
  ```
This leader's location is also where the origin of our spline actor should be. When we scale it up, the scale will then be uniform and the leader's location will stay the same.

## How to move the formation?
Because of the starting project, the movement logic for the player and the zombies are in Blueprints. To be able to finish this research I decided to do this logic in Blueprints as well.

In Blueprints we implement two parts.
### 1. Clicking on a location
Whenever you click on a location, move the formation there and tell the zombies to move to the formation locations.

![](https://github.com/StassijnsSam/Formations/blob/main/GIFs/MoveZombiesToFormation.gif)

### 2. Clicking on the player
Whenever you click on the player, the formation not only gets centered on them, but it also gets attached to them.  

![](https://github.com/StassijnsSam/Formations/blob/main/GIFs/MoveZombiesWithActor.gif)

Whenever the player moves, tell the zombies to move to the formation locations.

![](https://github.com/StassijnsSam/Formations/blob/main/GIFs/Moving.gif)

This is a very rudimentary implementation to show the potential of this research.

# Improvements
## 1. Reworking Blueprints to C++
The movement logic of the units is currently done with Blueprints. I would like to update them so that they completely work with A* pathfinding as well as some flocking.  
Since I already implemented these things in projects throughout the year, this would be the first thing I would add.

## 2. Formation movement logic
While doing research I came across a lot of the issues regular formations have that would also apply to this project.

### 2.1 Staying together when going around obstacles
It is possible the group splits when moving around an obstacle because of the pathing algorithm. In general though you would want your group to stay together and take the same path.

### 2.2 Letting other units pass
Sometimes units gets stuck on each other. By implementing a form of flocking, they would avoid each other enough for that not to happen.

### 3. Groups
Currently, you can only have one formation at a time. I am already working on adding groups and a group manager, so you can group up units and keep those groups in different formations as well. By combining those groups you would be able to make very advanced formations.

# Conclusion
I am very happy with what I have been able to achieve.  
As there wasn't any real research on ,or implementations of, drawable formations that I could find online, I have the feeling I was able to do something more unique. As a result, there was much trial and error involved in this project.  
I am planning to keep working on this project in the future.

# Sources
[Splines in Unreal Engine](http://jollymonsterstudio.com/2020/05/13/unreal-engine-c-fundamentals-using-spline-components/)

## Sources for future use
[Implementing coordinated movement](https://www.gamedeveloper.com/programming/implementing-coordinated-movement)  
[Coordinated movement](http://www.jeremiahwarm.com/CoordinatedMovement.php)  
[RTS group movement](https://sandruski.github.io/RTS-Group-Movement/)  
