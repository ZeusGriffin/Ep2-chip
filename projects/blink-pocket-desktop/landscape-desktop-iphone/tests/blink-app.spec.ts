import { expect, test } from "@playwright/test";

test("opens as a full iPhone app and keeps the reading canvas visible while typing", async ({ page }) => {
  await page.setViewportSize({ width: 844, height: 390 });
  await page.route("**/api/health", (route) => route.fulfill({ json: { ok: true, configured: true } }));
  await page.route("**/api/chat", (route) => route.fulfill({ json: { text: "Hello from NODE" } }));
  await page.goto("/");

  await expect(page.locator(".phone-bezel")).toBeHidden();
  await expect(page.getByTestId("device-picker")).toBeHidden();
  await expect(page.getByTestId("desktop-shell")).toHaveCSS("width", "844px");
  await expect(page.getByText("AI READY")).toBeVisible();
  await expect(page.getByLabel("Saved conversations")).toBeVisible();

  await page.getByRole("button", { name: "New canvas" }).click();
  await page.getByRole("button", { name: "H", exact: true }).click();
  await page.getByRole("button", { name: "I", exact: true }).click();
  await expect(page.getByRole("textbox", { name: "Message draft" })).toContainText("hi");

  const canvasBeforeSend = await page.getByLabel("Conversation reading canvas").boundingBox();
  await page.getByRole("button", { name: "SEND", exact: true }).click();
  await expect(page.getByText("Hello from NODE")).toBeVisible();
  const canvasAfterSend = await page.getByLabel("Conversation reading canvas").boundingBox();

  expect(canvasBeforeSend?.height).toBeGreaterThan(150);
  expect(canvasAfterSend?.height).toBeGreaterThan(150);
  await expect(page.getByTestId("keyboard-dock")).toBeHidden();

  await page.reload();
  await expect(page.getByText("Hello from NODE")).toBeVisible();
  await expect(page.getByRole("button", { name: "01 hi" })).toBeVisible();
});

test("removes the desktop preview chrome on a portrait iPhone viewport", async ({ page }) => {
  await page.setViewportSize({ width: 390, height: 844 });
  await page.route("**/api/health", (route) => route.fulfill({ json: { ok: true, configured: false } }));
  await page.goto("/");

  await expect(page.locator(".phone-bezel")).toBeHidden();
  await expect(page.getByTestId("device-picker")).toBeHidden();
  await expect(page.getByTestId("desktop-shell")).toHaveCSS("width", "390px");
  await expect(page.getByText("CONNECT AI")).toBeVisible();
});
