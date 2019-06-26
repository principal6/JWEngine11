#pragma once

// Math functions for DirectX11

namespace JWEngine 
{
	using namespace DirectX;

	static const XMVECTOR KVectorZero = XMVectorZero();
	static const XMVECTOR KVectorMax = XMVectorSet(D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX);
	static const XMVECTOR KVectorOne = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);

	static const XMMATRIX KMatrixIdentity = XMMatrixIdentity();

	static auto __vectorcall GetRayDirection(const XMVECTOR& RayOrigin, const XMVECTOR& PointInRayDirection)->XMVECTOR
	{
		return XMVector3Normalize(PointInRayDirection - RayOrigin);
	}
	
	static auto __vectorcall GetTriangleNormal(const XMVECTOR& EdgeAB, const XMVECTOR& EdgeAC)->XMVECTOR
	{
		return XMVector3Normalize(XMVector3Cross(EdgeAB, EdgeAC));
	}

	static auto __vectorcall GetTriangleNormal(const XMVECTOR& TriA, const XMVECTOR& TriB, const XMVECTOR& TriC)->XMVECTOR
	{
		return XMVector3Normalize(XMVector3Cross(TriB - TriA, TriC - TriA));
	}
	
	// Returns distance and projected point vector
	static auto __vectorcall ProjectPointOntoPlane(
		float& OutDistance, const XMVECTOR& Point, const XMVECTOR& PlanePoint, const XMVECTOR& PlaneNormal)->XMVECTOR
	{
		auto dist = XMVector3Dot(Point - PlanePoint, PlaneNormal);
		OutDistance = XMVectorGetX(dist);

		return (Point - PlaneNormal * dist);
	}

	static auto __vectorcall ProjectPointOntoPlane(
		const XMVECTOR& Point, const XMVECTOR& PlanePoint, const XMVECTOR& PlaneNormal)->XMVECTOR
	{
		auto dist = XMVector3Dot(Point - PlanePoint, PlaneNormal);

		return (Point - PlaneNormal * dist);
	}
	
	static auto __vectorcall IsPointOnPlaneInsideTriangle(
		const XMVECTOR& PointOnPlane, const XMVECTOR& TriA, const XMVECTOR& TriB, const XMVECTOR& TriC)->bool
	{
		auto edge_bc = TriC - TriB;
		auto edge_ac = TriC - TriA;
		auto edge_ab = TriB - TriA;

		auto check_0 = XMVector3Cross(edge_bc, (PointOnPlane - TriB));
		auto check_1 = XMVector3Cross(edge_bc, (TriA - TriB));
		{
			check_0 = XMVector3Cross(edge_ac, (PointOnPlane - TriA));
			check_1 = XMVector3Cross(edge_ac, edge_ab);
			if (XMVector3GreaterOrEqual(XMVector3Dot(check_0, check_1), KVectorZero))
			{
				check_0 = XMVector3Cross(edge_ab, (PointOnPlane - TriA));
				check_1 = XMVector3Cross(edge_ab, edge_ac);
				if (XMVector3GreaterOrEqual(XMVector3Dot(check_0, check_1), KVectorZero))
				{
					return true;
				}
			}
		}

		return false;
	}

	// #0 Plane equation
	// Ax + By + Cz + D = 0
	// => A, B, C = Face normal's xyz coord
	// => x, y, z = Any point in the plane, so we can just use V0
	// => Ax + By + Cz = Dot(normal, point) = Dot(N, P)
	// => D = -(Ax + By + Cz) = -Dot(normal, v0) = -Dot(N, V0)
	//
	//
	// #1 Ray equation (which is a parametric equation of a line)
	// L = P0 + t * P1
	//   = ray_origin + t * ray_direction
	//
	// @ N: plane normal  @ V = given vertex in the plane  @ L = line vector
	//
	// L = (P0x + P1x * t, P0y + P1y * t, P0z + P1z * t)
	//
	// Dot(N, L) = Dot(N, V)
	// => (P0x + P1x * t) * Nx + (P0y + P1y * t) * Ny + (P0z + P1z * t) * Nz = (Vx) * Nx + (Vy) * Ny + (Vz) * Nz
	// => (P1x * t) * Nx + (P1y * t) * Ny + (P1z * t) * Nz = (Vx - P0x) * Nx + (Vy - P0y) * Ny + (Vz - P0z) * Nz
	// => (P1x * Nx) * t + (P1y * Ny) * t + (P1z * Nz) * t = (Vx - P0x) * Nx + (Vy - P0y) * Ny + (Vz - P0z) * Nz
	// => Dot(P1, N) * t = Dot(V, N) - Dot(P0, N)
	//
	//           Dot(V, N) - Dot(P0, N)
	// =>  t  = ------------------------
	//                 Dot(P1, N)
	static auto __vectorcall IntersectRayTriangle(XMVECTOR& OutPointOnPlane, XMVECTOR& OutOldT,
		const XMVECTOR& RayOrigin, const XMVECTOR& RayDirection,
		const XMVECTOR& TriA, const XMVECTOR& TriB, const XMVECTOR& TriC) noexcept->bool
	{
		auto triangle_normal = GetTriangleNormal(TriA, TriB, TriC);
		auto plane_d = -XMVector3Dot(TriA, triangle_normal);

		auto ray_origin_norm = XMVector3Dot(RayOrigin, triangle_normal);
		auto ray_direction_norm = XMVector3Dot(RayDirection, triangle_normal);
		XMVECTOR new_t{};
		if (XMVector3NotEqual(ray_direction_norm, KVectorZero))
		{
			new_t = (-plane_d -ray_origin_norm) / ray_direction_norm;
		}

		// 't' should be positive for the picking to be in front of the camera!
		// (if it's negative, the picking is occuring behind the camera)
		// We will store the minimum of t values, which means that it's the closest picking to the camera.
		if ((XMVector3Greater(new_t, KVectorZero)) && (XMVector3Less(new_t, OutOldT)))
		{
			// Save the point on plane.
			OutPointOnPlane = RayOrigin + new_t * RayDirection;

			// Check if the point is in the triangle
			if (IsPointOnPlaneInsideTriangle(OutPointOnPlane, TriA, TriB, TriC))
			{
				//OutPointOnPlane = RayOrigin + new_t * RayDirection;
				OutOldT = new_t;
				return true;
			}
		}
		else
		{
			// Save the point on plane.
			OutPointOnPlane = RayOrigin + OutOldT * RayDirection;
		}

		return false;
	}

	static auto __vectorcall IntersectRayTriangle(XMVECTOR& OutPointOnPlane, const XMVECTOR& RayOrigin, const XMVECTOR& RayDirection,
		const XMVECTOR& TriA, const XMVECTOR& TriB, const XMVECTOR& TriC) noexcept->bool
	{
		auto triangle_normal = GetTriangleNormal(TriA, TriB, TriC);
		auto plane_d = -XMVector3Dot(TriA, triangle_normal);

		auto ray_origin_norm = XMVector3Dot(RayOrigin, triangle_normal);
		auto ray_direction_norm = XMVector3Dot(RayDirection, triangle_normal);
		XMVECTOR new_t{};
		if (XMVector3NotEqual(ray_direction_norm, KVectorZero))
		{
			new_t = (-plane_d - ray_origin_norm) / ray_direction_norm;
		}

		// 't' should be positive for the picking to be in front of the camera!
		// (if it's negative, the picking is occuring behind the camera)
		// We will store the minimum of t values, which means that it's the closest picking to the camera.
		if (XMVector3Greater(new_t, KVectorZero))
		{
			// Save the point on plane.
			OutPointOnPlane = RayOrigin + new_t * RayDirection;

			// Check if the point is in the triangle
			if (IsPointOnPlaneInsideTriangle(OutPointOnPlane, TriA, TriB, TriC))
			{
				return true;
			}
		}

		return false;
	}

	// Sphere equation
	// (Px - Cx)©÷ + (Py - Cy)©÷ + (Pz - Cz)©÷ = r©÷
	// Dot(P - C, P - C) = r©÷
	// # P = any point on the sphere  # C = center of the sphere  # r = radius of the sphere
	//
	// Line's parametric equation
	// L = O + tD
	// (L for Line, O for Ray Origin, D for Ray Direction)
	//
	// Line-Sphere intersection (which is the simultaneous equation of the two equations)
	// Dot(L - C, L - C) = r©÷
	// Dot(O + tD - C, O + tD - C) = r©÷
	// (Ox + t*Dx - Cx)©÷ + (Oy + t*Dy - Cy)©÷ + (Oz + t*Dz - Cz)©÷ = r©÷
	// (Ox + t*Dx - Cx)©÷ + (Oy + t*Dy - Cy)©÷ + (Oz + t*Dz - Cz)©÷ - r©÷ = 0
	//
	// Now, let's expand the equation
	// Ox©÷ + t©÷Dx©÷ + Cx©÷ + 2tOxDx -2OxCx -2tDxCx + Oy©÷ + t©÷Dy©÷ + Cy©÷ + 2tOyDy -2OyCy -2tDyCy
	//  + Oz©÷ + t©÷Dz©÷ + Cz©÷ + 2tOzDz -2OzCz -2tDzCz - r©÷ = 0
	//
	// Let's simplify it using Dot()
	// Dot(O, O) + t©÷Dot(D, D) + Dot(C, C) + 2tDot(O, D) - 2Dot(O, C) - 2tDot(D, C) - r©÷ = 0
	//
	// Let's rearrange it by 't'
	// t©÷Dot(D, D) + 2t(Dot(D, O) - Dot(D, C)) + Dot(O, O) - 2Dot(O, C) + Dot(C, C) - r©÷ = 0
	//
	// Optimize Dot() #0
	// t©÷Dot(D, D) + 2t(Dot(D, O - C)) + Dot(O, O) - 2Dot(O, C) + Dot(C, C) - r©÷ = 0
	//
	// Optimize Dot() #1
	// t©÷Dot(D, D) + 2t(Dot(D, O - C)) + Dot(O - C, O - C) - r©÷ = 0
	//
	// That wil give us the following quadratic equation
	// at©÷ + bt + c = 0
	// # a = Dot(D, D)  # b = 2 * Dot(D, O - C)  # c = Dot(O - C, O - C) - r©÷
	//
	// discriminant of quadratic equation: b©÷ - 4ac
	//
	// And if, b©÷ - 4ac ¡Ã 0
	// then, the ray hit the sphere!
	static auto __vectorcall IntersectRaySphere(
		const XMVECTOR& RayOrigin, const XMVECTOR& RayDirection, float Radius, const XMVECTOR& Center,
		XMVECTOR* PtrOutOldT = nullptr) noexcept->bool
	{
		auto r = XMVectorSet(Radius, Radius, Radius, 1.0f);
		auto vector_co = RayOrigin - Center;

		auto a = XMVector3Dot(RayDirection, RayDirection);
		auto b = 2.0f * XMVector3Dot(RayDirection, vector_co);
		auto c = XMVector3Dot(vector_co, vector_co) - r * r;
		auto discriminant = b * b - 4.0f * a * c;

		if (XMVector3Greater(discriminant, KVectorZero))
		{
			// Smaller T
			auto new_t = XMVectorSetW((-b -XMVectorSqrt(discriminant)) / (2 * a), 1.0f);

			if (XMVector3Less(new_t, KVectorZero))
			{
				// Bigger T
				new_t = XMVectorSetW((-b +XMVectorSqrt(discriminant)) / (2 * a), 1.0f);
			}

			if (PtrOutOldT)
			{
				if ((XMVector3Greater(new_t, KVectorZero)) && (XMVector3Less(new_t, *PtrOutOldT)))
				{
					*PtrOutOldT = new_t;
					return true;
				}
			}
			else
			{
				if (XMVector3Greater(new_t, KVectorZero))
				{
					return true;
				}
			}
			
		}

		return false;
	}

	static auto __vectorcall IntersectRayUnitSphere(
		const XMVECTOR& RayOrigin, const XMVECTOR& RayDirection, XMVECTOR* PtrOutOldT = nullptr) noexcept->bool
	{
		auto r = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		auto vector_co = RayOrigin;

		auto a = XMVector3Dot(RayDirection, RayDirection);
		auto b = 2.0f * XMVector3Dot(RayDirection, vector_co);
		auto c = XMVector3Dot(vector_co, vector_co) - r * r;
		auto discriminant = b * b - 4.0f * a * c;

		if (XMVector3Greater(discriminant, KVectorZero))
		{
			// Smaller T
			auto new_t = XMVectorSetW((-b - XMVectorSqrt(discriminant)) / (2 * a), 1.0f);

			if (XMVector3Less(new_t, KVectorZero))
			{
				// Bigger T
				new_t = XMVectorSetW((-b + XMVectorSqrt(discriminant)) / (2 * a), 1.0f);
			}

			if (PtrOutOldT)
			{
				if ((XMVector3Greater(new_t, KVectorZero)) && (XMVector3Less(new_t, *PtrOutOldT)))
				{
					*PtrOutOldT = new_t;
					return true;
				}
			}
			else
			{
				if (XMVector3Greater(new_t, KVectorZero))
				{
					return true;
				}
			}

		}

		return false;
	}

	// @warning: NOT TESTED YET
	static auto __vectorcall IntersectPlaneSphere(
		const XMVECTOR& PlaneNormal, const XMVECTOR& PlanePoint, float Radius, const XMVECTOR& Center, float* OutDistancePtr = nullptr) noexcept->bool
	{
		auto relative_position = Center - PlanePoint;
		auto distance = XMVector3Dot(relative_position, PlaneNormal);

		if (XMVectorGetX(distance) >= Radius)
		{
			return true;
		}

		return false;
	}

	static auto __vectorcall IntersectSpheres(
		float RadiusA, const XMVECTOR& CenterA, float RadiusB, const XMVECTOR& CenterB, float* OutSqaureDistancePtr = nullptr) noexcept->bool
	{
		auto distance_vector = CenterA - CenterB;
		distance_vector = XMVector3Dot(distance_vector, distance_vector);
		auto square_distance = XMVectorGetX(distance_vector);

		auto sqaure_radii = RadiusA + RadiusB;
		sqaure_radii *= sqaure_radii;

		if (OutSqaureDistancePtr)
		{
			*OutSqaureDistancePtr = square_distance;
		}

		if (square_distance <= sqaure_radii)
		{
			return true;
		}

		return false;
	}

	static auto __vectorcall GetDistanceBetweenPointAndLine(
		const XMVECTOR& Point, const XMVECTOR& LinePointA, const XMVECTOR& LinePointB) noexcept->float
	{
		auto ab_dir = XMVector3Normalize(LinePointB - LinePointA);
		auto ap = Point - LinePointA;
		auto p_proj = XMVector3Dot(ap, ab_dir) * ab_dir + LinePointA;
		auto p_perp = Point - p_proj;

		return XMVectorGetX(XMVector3Length(p_perp));
	}

	static auto __vectorcall ProjectPointOntoSegment(
		const XMVECTOR& Point, const XMVECTOR& SegmentA, const XMVECTOR& SegmentB, XMVECTOR& ProjectedPoint)->bool
	{
		auto seg = SegmentA - SegmentB;
		auto seg_dir = XMVector3Normalize(seg);
		auto v = Point - SegmentB;

		auto projected_v = XMVector3Dot(v, seg_dir) * seg_dir;
		auto projected_dir = XMVector3Normalize(projected_v);
		ProjectedPoint = SegmentB + projected_v;

		auto seg_l_sq = XMVector3LengthSq(seg);
		auto projected_l_sq = XMVector3LengthSq(projected_v);

		if (XMVector3NotEqual(seg_dir, projected_dir))
		{
			return false;
		}

		if (XMVector3Greater(projected_l_sq, seg_l_sq))
		{
			return false;
		}
		else
		{
			return true;
		}
	}
};
