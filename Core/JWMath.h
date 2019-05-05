#pragma once

// Math functions for DirectX11

namespace JWEngine 
{
	using namespace DirectX;

	static const auto KZeroVector = XMVectorZero();

	__forceinline auto __vectorcall GetTriangleNormal(const XMVECTOR& EdgeAB, const XMVECTOR& EdgeAC)->XMVECTOR
	{
		return XMVector3Normalize(XMVector3Cross(EdgeAB, EdgeAC));
	}

	__forceinline auto __vectorcall GetTriangleNormal(const XMVECTOR& TriA, const XMVECTOR& TriB, const XMVECTOR& TriC)->XMVECTOR
	{
		return XMVector3Normalize(XMVector3Cross(TriB - TriA, TriC - TriA));
	}

	// Returns distance and projected point vector
	__forceinline auto __vectorcall ProjectPointOntoPlane(
		float& OutDistance, const XMVECTOR& Point, const XMVECTOR& PlanePoint, const XMVECTOR& PlaneNormal)->XMVECTOR
	{
		auto dist = XMVector3Dot(Point - PlanePoint, PlaneNormal);
		OutDistance = XMVectorGetX(dist);

		return (Point - PlaneNormal * dist);
	}

	__forceinline auto __vectorcall IsPointInsideTriangle(
		const XMVECTOR& Point, const XMVECTOR& TriA, const XMVECTOR& TriB, const XMVECTOR& TriC)->bool
	{
		auto edge_bc = TriC - TriB;
		auto edge_ac = TriC - TriA;
		auto edge_ab = TriB - TriA;

		auto check_0 = XMVector3Cross(edge_bc, (Point - TriB));
		auto check_1 = XMVector3Cross(edge_bc, (TriA - TriB));
		if (XMVector3Greater(XMVector3Dot(check_0, check_1), KZeroVector))
		{
			check_0 = XMVector3Cross(edge_ac, (Point - TriA));
			check_1 = XMVector3Cross(edge_ac, edge_ab);
			if (XMVector3Greater(XMVector3Dot(check_0, check_1), KZeroVector))
			{
				check_0 = XMVector3Cross(edge_ab, (Point - TriA));
				check_1 = XMVector3Cross(edge_ab, edge_ac);
				if (XMVector3Greater(XMVector3Dot(check_0, check_1), KZeroVector))
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
	__forceinline auto __vectorcall IntersectRayTriangle(XMVECTOR& OutPointOnPlane, XMVECTOR& OutOldT,
		const XMVECTOR& RayOrigin, const XMVECTOR& RayDirection,
		const XMVECTOR& TriA, const XMVECTOR& TriB, const XMVECTOR& TriC) noexcept->bool
	{
		auto triangle_normal = GetTriangleNormal(TriA, TriB, TriC);
		auto plane_d = -XMVector3Dot(TriA, triangle_normal);

		auto ray_origin_norm = XMVector3Dot(RayOrigin, triangle_normal);
		auto ray_direction_norm = XMVector3Dot(RayDirection, triangle_normal);
		XMVECTOR new_t{};
		if (XMVector3NotEqual(ray_direction_norm, KZeroVector))
		{
			new_t = (-plane_d -ray_origin_norm) / ray_direction_norm;
		}

		// 't' should be positive for the picking to be in front of the camera!
		// (if it's negative, the picking is occuring behind the camera)
		// We will store the minimum of t values, which means that it's the closest picking to the camera.
		if ((XMVector3Greater(new_t, KZeroVector)) && (XMVector3Less(new_t, OutOldT)))
		{
			// Save the point on plane.
			OutPointOnPlane = RayOrigin + new_t * RayDirection;

			// Check if the point is in the triangle
			if (IsPointInsideTriangle(OutPointOnPlane, TriA, TriB, TriC))
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
	__forceinline auto __vectorcall IntersectRaySphere(
		const XMVECTOR& RayOrigin, const XMVECTOR& RayDirection, const XMVECTOR& Center, float Radius) noexcept->bool
	{
		auto r = XMVectorSet(Radius, Radius, Radius, 1.0f);
		auto vector_co = RayOrigin - Center;

		auto a = XMVector3Dot(RayDirection, RayDirection);
		auto b = 2.0f * XMVector3Dot(RayDirection, vector_co);
		auto c = XMVector3Dot(vector_co, vector_co) - r * r;
		auto discriminant = b * b - 4.0f * a * c;

		if (XMVector3GreaterOrEqual(discriminant, KZeroVector))
		{
			return true;
		}

		return false;
	}
};
