const state = { products: [], cart: [], coupon: "" };
const MAX_QTY = 1000;
const money = (n) => `₹${Number(n).toFixed(2)}`;

// Page me daalne se pehle text ko saaf karta hai (HTML injection se bachav)
function esc(value) {
  return String(value).replace(
    /[&<>"']/g,
    (c) =>
      ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" })[
        c
      ],
  );
}

function notify(text) {
  const el = document.querySelector("#message");
  el.textContent = text;
  el.classList.add("show");
  setTimeout(() => el.classList.remove("show"), 3000);
}

// Server se baat karta hai; network error ya galat jawab par bhi crash nahi hota
async function api(path, options) {
  try {
    const response = await fetch(path, options);
    let data = null;
    try {
      data = await response.json();
    } catch {
      data = null;
    }
    return { ok: response.ok, data };
  } catch {
    return {
      ok: false,
      data: { error: "Cannot reach the server. Is EShopWeb.exe running?" },
    };
  }
}

function postJson(path, body) {
  return api(path, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(body),
  });
}

async function loadProducts() {
  const { ok, data } = await api("/api/products");
  if (!ok || !Array.isArray(data)) {
    document.querySelector("#catalog").innerHTML =
      "<p>Could not load products. Is the server running?</p>";
    notify((data && data.error) || "Could not load products.");
    return;
  }
  state.products = data;
  // Jo product ab catalog me nahi hai use cart se hata do
  state.cart = state.cart.filter((item) =>
    state.products.some((p) => p.id === item.id),
  );
  renderProducts();
  renderCart();
}

function detail(p) {
  if (p.type === "Electronics" && p.extra) return `Warranty: ${p.extra} months`;
  if (p.type === "Clothing" && p.extra) return `Size: ${p.extra}`;
  return "";
}

function renderProducts() {
  const el = document.querySelector("#catalog");
  if (!state.products.length) {
    el.innerHTML =
      "<p>Inventory is empty. Please ask an admin to add products.</p>";
    return;
  }
  el.innerHTML = state.products
    .map((p) => {
      const info = detail(p);
      return `<article class="card"><small>${esc(p.type)}</small><h3>${esc(p.name)}</h3>${
        info ? `<p style="margin:0 0 8px;color:#666">${esc(info)}</p>` : ""
      }<div class="price">${money(p.price)}</div><button onclick="add(${Number(p.id)})">Add to cart</button></article>`;
    })
    .join("");
}

function add(id) {
  const item = state.cart.find((x) => x.id === id);
  if (item) {
    if (item.quantity >= MAX_QTY) return notify(`Maximum ${MAX_QTY} per item.`);
    item.quantity++;
  } else state.cart.push({ id, quantity: 1 });
  renderCart();
}

function change(id, delta) {
  const item = state.cart.find((x) => x.id === id);
  if (!item) return;
  item.quantity += delta;
  if (item.quantity > MAX_QTY) {
    item.quantity = MAX_QTY;
    notify(`Maximum ${MAX_QTY} per item.`);
  }
  if (item.quantity < 1) state.cart = state.cart.filter((x) => x.id !== id);
  renderCart();
}

// C++ ke ShoppingCart::computeDiscount() jaisa hi hisaab (dono jagah ek jaise rakhna)
function discountFor(subtotal, coupon) {
  let percent = 0;
  if (coupon === "SAVE10") percent = 10;
  else if (coupon === "SAVE20" && subtotal >= 1000) percent = 20;
  else if (subtotal >= 1000) percent = 15;
  else if (subtotal >= 500) percent = 10;
  return Math.round(subtotal * percent) / 100;
}

function renderCart() {
  let subtotal = 0,
    count = 0;
  const rows = [];
  for (const item of state.cart) {
    const p = state.products.find((x) => x.id === item.id);
    if (!p) continue;
    subtotal += p.price * item.quantity;
    count += item.quantity;
    rows.push(
      `<div class="cart-row"><div><b>${esc(p.name)}</b><br>${money(p.price)} × ${item.quantity}</div><div class="qty"><button onclick="change(${Number(p.id)},-1)">−</button>${item.quantity}<button onclick="change(${Number(p.id)},1)">+</button></div></div>`,
    );
  }
  document.querySelector("#cartItems").innerHTML =
    rows.join("") || "<p>Your cart is empty.</p>";
  const discount = discountFor(subtotal, state.coupon);
  document.querySelector("#cartCount").textContent =
    `${count} item${count === 1 ? "" : "s"}`;
  document.querySelector("#subtotal").textContent = money(subtotal);
  document.querySelector("#discount").textContent = `-${money(discount)}`;
  document.querySelector("#total").textContent = money(subtotal - discount);
}

// Admin form: type ke hisaab se last khaana (warranty ya size) badalta hai
function updateExtraField() {
  const type = document.querySelector("#productForm [name=type]").value;
  const extra = document.querySelector("#productForm [name=extra]");
  extra.value = "";
  if (type === "1") {
    extra.type = "number";
    extra.min = "0";
    extra.max = "600";
    extra.step = "1";
    extra.placeholder = "Warranty (months)";
  } else {
    extra.type = "text";
    extra.removeAttribute("min");
    extra.removeAttribute("max");
    extra.removeAttribute("step");
    extra.maxLength = 10;
    extra.placeholder = "Size (S / M / L)";
  }
}

document.querySelector("#coupon").oninput = (e) => {
  state.coupon = e.target.value.trim().toUpperCase();
  renderCart();
};
document.querySelector("#refresh").onclick = loadProducts;
document.querySelector("#adminToggle").onclick = () => {
  const admin = document.querySelector("#admin");
  admin.classList.toggle("hidden");
  if (!admin.classList.contains("hidden")) {
    admin.scrollIntoView({ behavior: "smooth", block: "center" });
  }
};
document.querySelector("#closeModal").onclick = () =>
  document.querySelector("#modal").classList.add("hidden");
document.querySelector("#checkout").onclick = () => {
  if (!state.cart.length) return notify("Add a product before checkout.");
  document.querySelector("#modal").classList.remove("hidden");
};
document.querySelector("#productForm [name=type]").onchange = updateExtraField;

document.querySelector("#productForm").onsubmit = async (e) => {
  e.preventDefault();
  const form = Object.fromEntries(new FormData(e.target));
  form.name = String(form.name || "").trim();
  form.extra = String(form.extra || "").trim();
  if (/["\\|]/.test(form.name) || /["\\|]/.test(form.extra))
    return notify('Name and size cannot contain " \\ or |');
  const button = e.target.querySelector("button.primary");
  button.disabled = true;
  try {
    const { ok, data } = await postJson("/api/products", form);
    if (!ok) return notify((data && data.error) || "Could not add product.");
    notify("Product added");
    e.target.reset();
    updateExtraField();
    await loadProducts();
  } finally {
    button.disabled = false;
  }
};

document.querySelector("#checkoutForm").onsubmit = async (e) => {
  e.preventDefault();
  const fields = Object.fromEntries(new FormData(e.target));
  const { card, ...customer } = fields; // card sirf ek baar bheja jata hai
  const button = e.target.querySelector("button.primary");
  button.disabled = true;
  try {
    const { ok, data } = await postJson("/api/checkout", {
      customer,
      card: String(card || "").replace(/\s/g, ""),
      coupon: state.coupon,
      items: state.cart,
    });
    if (!ok) return notify((data && data.error) || "Checkout failed.");
    document.querySelector("#modal").classList.add("hidden");
    e.target.reset();
    state.cart = [];
    state.coupon = "";
    document.querySelector("#coupon").value = "";
    renderCart();
    notify(`Payment successful. ${data.receipt} generated.`);
  } finally {
    button.disabled = false;
  }
};

updateExtraField();
loadProducts();
renderCart();